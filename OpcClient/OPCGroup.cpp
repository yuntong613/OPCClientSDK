/*
OPCClientToolKit
Copyright (C) 2005 Mark C. Beharrell

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"
#include "OPCServer.h"
#include "OPCGroup.h"
#include "OPCItem.h"
#include "OPCException.h"


/**
* Handles OPC (DCOM) callbacks at the group level. It deals with the receipt of data from asynchronous operations.
* This is a fake COM object.
*/
class CAsynchDataCallback : public IOPCDataCallback
{
private:
	DWORD mRefCount;
	/**
	* group this is a callback for
	*/
	COPCGroup &callbacksGroup;

public:
	CAsynchDataCallback(COPCGroup &group) :callbacksGroup(group) {
		mRefCount = 0;
	}

	~CAsynchDataCallback() {
	}

	/**
	* Functions associated with IUNKNOWN
	*/
	STDMETHODIMP QueryInterface(REFIID iid, LPVOID* ppInterface) {
		if (ppInterface == NULL) {
			return E_INVALIDARG;
		}

		if (iid == IID_IUnknown) {
			*ppInterface = (IUnknown*)this;
		}
		else if (iid == IID_IOPCDataCallback) {
			*ppInterface = (IOPCDataCallback*)this;
		}
		else
		{
			*ppInterface = NULL;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}


	STDMETHODIMP_(ULONG) AddRef() {
		return ++mRefCount;
	}

	STDMETHODIMP_(ULONG) Release() {
		--mRefCount;

		if (mRefCount == 0) {
			delete this;
		}
		return mRefCount;
	}

	/**
	* Functions associated with IOPCDataCallback
	*/
	STDMETHODIMP OnDataChange(DWORD Transid, OPCHANDLE grphandle, HRESULT masterquality,
		HRESULT mastererror, DWORD count, OPCHANDLE * clienthandles,
		VARIANT * values, WORD * quality, FILETIME  * time,
		HRESULT * errors)
	{
		IAsynchDataCallback * usrHandler = callbacksGroup.getUsrAsynchHandler();

		if (Transid != 0) {
			// it is a result of a refresh (see p106 of spec)
			CTransaction & trans = *(CTransaction *)Transid;
			updateOPCData(trans.opcData, count, clienthandles, values, quality, time, errors);
			trans.setCompleted();
			return S_OK;
		}

		if (usrHandler) {
			COPCItem_DataMap dataChanges;
			updateOPCData(dataChanges, count, clienthandles, values, quality, time, errors);
			usrHandler->OnDataChange(callbacksGroup, dataChanges);
		}
		return S_OK;
	}

	STDMETHODIMP OnReadComplete(DWORD Transid, OPCHANDLE grphandle,
		HRESULT masterquality, HRESULT mastererror, DWORD count,
		OPCHANDLE * clienthandles, VARIANT* values, WORD * quality,
		FILETIME * time, HRESULT * errors)
	{
		// TODO this is bad  - server could corrupt address - need to use look up table
		CTransaction & trans = *(CTransaction *)Transid;
		updateOPCData(trans.opcData, count, clienthandles, values, quality, time, errors);
		trans.setCompleted();
		return S_OK;
	}

	STDMETHODIMP OnWriteComplete(DWORD Transid, OPCHANDLE grphandle, HRESULT mastererr,
		DWORD count, OPCHANDLE * clienthandles, HRESULT * errors)
	{
		// TODO this is bad  - server could corrupt address - need to use look up table
		CTransaction & trans = *(CTransaction *)Transid;

		// see page 145 - number of items returned may be less than sent
		for (unsigned i = 0; i < count; i++) {
			// TODO this is bad  - server could corrupt address - need to use look up table
			COPCItem * item = (COPCItem *)clienthandles[i];
			trans.setItemError(item, errors[i]); // this records error state - may be good
		}

		trans.setCompleted();
		return S_OK;
	}

	STDMETHODIMP OnCancelComplete(DWORD transid, OPCHANDLE grphandle) {
		printf("OnCancelComplete: Transid=%ld GrpHandle=%ld\n", transid, grphandle);
		return S_OK;
	}

	/**
	* make OPC item
	*/
	static OPCItemData * makeOPCDataItem(VARIANT& value, WORD quality, FILETIME & time, HRESULT error) {
		OPCItemData * data = NULL;
		if (FAILED(error)) {
			data = new OPCItemData(error);
		}
		else {
			data = new OPCItemData(time, quality, value, error);
		}
		return data;
	}

	/**
	* Enter the OPC items data that resulted from an operation
	*/
	static void updateOPCData(COPCItem_DataMap &opcData, DWORD count, OPCHANDLE * clienthandles,
		VARIANT* values, WORD * quality, FILETIME * time, HRESULT * errors) {
		// see page 136 - returned arrays may be out of order
		for (unsigned i = 0; i < count; i++) {
			// TODO this is bad  - server could corrupt address - need to use look up table
			COPCItem * item = (COPCItem *)clienthandles[i];
			OPCItemData * data = makeOPCDataItem(values[i], quality[i], time[i], errors[i]);
			COPCItem_DataMap::CPair* pair = opcData.Lookup(item);
			if (pair == NULL) {
				opcData.SetAt(item, data);
			}
			else {
				opcData.SetValueAt(pair, data);
			}
		}
	}
};

COPCGroup::COPCGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand, COPCServer &server) :
	name(groupName),
	opcServer(server)
{
	std::wstring  wideName = CUtils::ANSIToUnicode(groupName);

	HRESULT result = opcServer.getServerInterface()->AddGroup(wideName.c_str(), active, reqUpdateRate_ms, 0, 0, &deadBand,
		0, &groupHandle, &revisedUpdateRate_ms, IID_IOPCGroupStateMgt, (LPUNKNOWN*)&iStateManagement);
	if (FAILED(result))
	{
		throw OPCException("Failed to Add group", result);
	}

	result = iStateManagement->QueryInterface(IID_IOPCSyncIO, (void**)&iSychIO);
	if (FAILED(result)) {
		throw OPCException("Failed to get IID_IOPCSyncIO", result);
	}

	result = iStateManagement->QueryInterface(IID_IOPCAsyncIO2, (void**)&iAsych2IO);
	if (FAILED(result)) {
		throw OPCException("Failed to get IID_IOPCAsyncIO2", result);
	}

	result = iStateManagement->QueryInterface(IID_IOPCItemMgt, (void**)&iItemManagement);
	if (FAILED(result)) {
		throw OPCException("Failed to get IID_IOPCItemMgt", result);
	}
}

COPCGroup::~COPCGroup()
{
	RemoveAllItems();
	opcServer.getServerInterface()->RemoveGroup(groupHandle, FALSE);
}

void COPCGroup::AddItemToMap(COPCItem* pItem)
{
	std::lock_guard<std::mutex> itemLock(m_ItemLock);

	std::map<std::string, COPCItem*>::iterator it = m_mapItems.find(pItem->getName());
	if (it != m_mapItems.end())
	{
		COPCItem* pItemFind = it->second;
		if (pItemFind)
		{
			delete pItemFind;
			pItemFind = NULL;
		}
	}
	m_mapItems.insert(std::pair<std::string,COPCItem*>(pItem->getName(), pItem));
}

void COPCGroup::RemoveItemFromMap(const char* itemName)
{
	std::lock_guard<std::mutex> itemLock(m_ItemLock);

	std::map<std::string, COPCItem*>::iterator it = m_mapItems.find(itemName);
	if (it != m_mapItems.end())
	{
		COPCItem* pItemFind = it->second;
		if (pItemFind)
		{
			delete pItemFind;
			pItemFind = NULL;
			m_mapItems.erase(it);
		}
	}
}

void COPCGroup::RemoveAllItems()
{
	std::lock_guard<std::mutex> itemLock(m_ItemLock);

	std::map<std::string, COPCItem*>::iterator it;
	while (!m_mapItems.empty())
	{
		it = m_mapItems.begin();
		COPCItem* pItemFind = it->second;
		if (pItemFind)
		{
			delete pItemFind;
			pItemFind = NULL;
		}
		m_mapItems.erase(it);
	}
	m_mapItems.clear();
}

OPCHANDLE * COPCGroup::buildServerHandleList(std::vector<COPCItem *>& items) {
	OPCHANDLE *handles = new OPCHANDLE[items.size()];
	for (unsigned i = 0; i < items.size(); i++) {
		if (items[i] == NULL) {
			delete[]handles;
			throw OPCException("Item is NULL");
		}
		handles[i] = items[i]->getHandle();
	}
	return handles;
}

void COPCGroup::readSync(std::vector<COPCItem *>& items, COPCItem_DataMap & opcData, OPCDATASOURCE source) {
	OPCHANDLE *serverHandles = buildServerHandleList(items);
	HRESULT *itemResult;
	OPCITEMSTATE *itemState;
	DWORD noItems = (DWORD)items.size();

	HRESULT	result = iSychIO->Read(source, noItems, serverHandles, &itemState, &itemResult);
	if (FAILED(result)) {
		delete[]serverHandles;
		throw OPCException("Read failed", result);
	}

	for (unsigned i = 0; i < noItems; i++) {
		COPCItem * item = (COPCItem *)itemState[i].hClient;
		OPCItemData * data = CAsynchDataCallback::makeOPCDataItem(itemState[i].vDataValue, itemState[i].wQuality, itemState[i].ftTimeStamp, itemResult[i]);
		COPCItem_DataMap::CPair* pair = opcData.Lookup(item);
		if (pair == NULL) {
			opcData.SetAt(item, data);
		}
		else {
			opcData.SetValueAt(pair, data);
		}
	}

	delete[]serverHandles;
	CoTaskMemFree(itemResult);
	CoTaskMemFree(itemState);
}

CTransaction * COPCGroup::readAsync(std::vector<COPCItem *>& items, ITransactionComplete *transactionCB) {
	DWORD cancelID;
	HRESULT * individualResults;
	CTransaction * trans = new CTransaction(items, transactionCB);
	OPCHANDLE *serverHandles = buildServerHandleList(items);
	DWORD noItems = (DWORD)items.size();

	HRESULT result = iAsych2IO->Read(noItems, serverHandles, (DWORD)trans, &cancelID, &individualResults);
	delete[] serverHandles;
	if (FAILED(result)) {
		delete trans;
		throw OPCException("Asynch Read failed",result);
	}

	trans->setCancelId(cancelID);
	unsigned failCount = 0;
	for (unsigned i = 0; i < noItems; i++) {
		if (FAILED(individualResults[i])) {
			trans->setItemError(items[i], individualResults[i]);
			failCount++;
		}
	}
	if (failCount == items.size()) {
		trans->setCompleted(); // if all items return error then no callback will occur. p 101
	}

	CoTaskMemFree(individualResults);
	return trans;
}

CTransaction * COPCGroup::refresh(OPCDATASOURCE source, ITransactionComplete *transactionCB) {
	DWORD cancelID;
	CTransaction * trans = new CTransaction(items, transactionCB);

	HRESULT result = iAsych2IO->Refresh2(source, (DWORD)trans, &cancelID);
	if (FAILED(result)) {
		delete trans;
		throw OPCException("refresh failed", result);
	}

	return trans;
}

COPCItem * COPCGroup::addItem(std::string &itemName, bool active)
{
	std::vector<std::string> names;
	std::vector<COPCItem *> itemsCreated;
	std::vector<HRESULT> errors;
	names.push_back(itemName);
	if (addItems(names, itemsCreated, errors, active) != 0) {
		throw OPCException("Failed to add item",errors.front());
	}
	return itemsCreated[0];
}

bool COPCGroup::WriteOPCValue(const char* itemName, VARIANT& vtValue)
{
	std::lock_guard<std::mutex> itemLock(m_ItemLock);

	std::map<std::string, COPCItem*>::iterator it = m_mapItems.find(itemName);
	if (it != m_mapItems.end())
	{
		COPCItem* pItemFind = it->second;
		if (pItemFind)
		{
			CTransaction* pTrans = pItemFind->writeAsynch(vtValue);
			if (pTrans)
			{
				delete pTrans;
				pTrans = NULL;
			}
		}
	}
	return false;
}

int COPCGroup::addItems(std::vector<std::string>& itemName, std::vector<COPCItem *>& itemsCreated, std::vector<HRESULT>& errors, bool active) {
	itemsCreated.resize(itemName.size());
	errors.resize(itemName.size());
	OPCITEMDEF *itemDef = new OPCITEMDEF[itemName.size()];
	unsigned i = 0;
	std::vector<CT2OLE *> tpm;
	for (; i < itemName.size(); i++) {
		itemsCreated[i] = new COPCItem(itemName[i], *this);
		USES_CONVERSION;
		tpm.push_back(new CT2OLE(itemName[i].c_str()));
		itemDef[i].szItemID = **(tpm.end() - 1);
		itemDef[i].szAccessPath = NULL;//wideName;
		itemDef[i].bActive = active;
		itemDef[i].hClient = (DWORD)itemsCreated[i];
		itemDef[i].dwBlobSize = 0;
		itemDef[i].pBlob = NULL;
		itemDef[i].vtRequestedDataType = VT_EMPTY;
	}

	HRESULT *itemResult;
	OPCITEMRESULT *itemDetails;
	DWORD noItems = (DWORD)itemName.size();

	HRESULT	result = getItemManagementInterface()->AddItems(noItems, itemDef, &itemDetails, &itemResult);
	delete[] itemDef;
	for (size_t i = 0; i < itemName.size(); i++) {
		delete tpm[i];
	}
	if (FAILED(result)) {
		throw OPCException("Failed to add items", result);
	}

	int errorCount = 0;
	for (i = 0; i < noItems; i++) {
		if (itemDetails[i].pBlob) {
			CoTaskMemFree(itemDetails[0].pBlob);
		}

		if (FAILED(itemResult[i])) {
			delete itemsCreated[i];
			itemsCreated[i] = NULL;
			errors[i] = itemResult[i];
			errorCount++;
		}
		else {
			(itemsCreated[i])->setOPCParams(itemDetails[i].hServer, itemDetails[i].vtCanonicalDataType, itemDetails[i].dwAccessRights);
			errors[i] = ERROR_SUCCESS;
		}
	}

	CoTaskMemFree(itemDetails);
	CoTaskMemFree(itemResult);

	return errorCount;
}

void COPCGroup::enableAsynch(IAsynchDataCallback &handler) {
	if (!asynchDataCallBackHandler == false) {
		throw OPCException("Asynch already enabled");
	}

	ATL::CComPtr<IConnectionPointContainer> iConnectionPointContainer = 0;
	HRESULT result = iStateManagement->QueryInterface(IID_IConnectionPointContainer, (void**)&iConnectionPointContainer);
	if (FAILED(result))
	{
		throw OPCException("Could not get IID_IConnectionPointContainer",result);
	}


	result = iConnectionPointContainer->FindConnectionPoint(IID_IOPCDataCallback, &iAsynchDataCallbackConnectionPoint);
	if (FAILED(result))
	{
		throw OPCException("Could not get IID_IOPCDataCallback",result);
	}


	asynchDataCallBackHandler = new CAsynchDataCallback(*this);
	result = iAsynchDataCallbackConnectionPoint->Advise(asynchDataCallBackHandler, &callbackHandle);
	if (FAILED(result))
	{
		iAsynchDataCallbackConnectionPoint = NULL;
		asynchDataCallBackHandler = NULL;
		throw OPCException("Failed to set DataCallbackConnectionPoint",result);
	}

	userAsynchCBHandler = &handler;
}

void COPCGroup::setState(DWORD reqUpdateRate_ms, DWORD &returnedUpdateRate_ms, float deadBand, BOOL active) {
	HRESULT result = iStateManagement->SetState(&reqUpdateRate_ms, &returnedUpdateRate_ms, &active, 0, &deadBand, 0, 0);
	if (FAILED(result))
	{
		throw OPCException("Failed to set group state",result);
	}
}

void COPCGroup::disableAsynch() {
	if (asynchDataCallBackHandler == NULL) {
		throw OPCException("Asynch is not exabled");
	}
	iAsynchDataCallbackConnectionPoint->Unadvise(callbackHandle);
	iAsynchDataCallbackConnectionPoint = NULL;
	asynchDataCallBackHandler = NULL;// WE DO NOT DELETE callbackHandler, let the COM ref counting take care of that
	if (userAsynchCBHandler)
	{
		delete userAsynchCBHandler;
		userAsynchCBHandler = NULL;
	}
}