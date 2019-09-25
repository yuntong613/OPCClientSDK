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
	COPCGroup& m_CallbacksGroup;

public:
	CAsynchDataCallback(COPCGroup& group) :m_CallbacksGroup(group) {
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
		HRESULT mastererror, DWORD count, OPCHANDLE* clienthandles,
		VARIANT* values, WORD* quality, FILETIME* time,
		HRESULT* errors)
	{
		IAsynchDataCallback* usrHandler = m_CallbacksGroup.getUsrAsynchHandler();

		if (Transid != 0) {
			// it is a result of a refresh (see p106 of spec)
			CTransaction* trans = m_CallbacksGroup.getServer().PeekTransaction(Transid);
			if (trans)
			{
				updateOPCData(trans->opcData, count, clienthandles, values, quality, time, errors);
				trans->setCompleted();
			}
			return S_OK;
		}

		if (usrHandler) {
			COPCItem_DataMap dataChanges;
			updateOPCData(dataChanges, count, clienthandles, values, quality, time, errors);
			usrHandler->OnDataChange(m_CallbacksGroup, dataChanges);
		}
		return S_OK;
	}

	STDMETHODIMP OnReadComplete(DWORD Transid, OPCHANDLE grphandle,
		HRESULT masterquality, HRESULT mastererror, DWORD count,
		OPCHANDLE* clienthandles, VARIANT* values, WORD* quality,
		FILETIME* time, HRESULT* errors)
	{
		// TODO this is bad  - server could corrupt address - need to use look up table

		CTransaction* trans = m_CallbacksGroup.getServer().PeekTransaction(Transid);
		if (trans)
		{
			updateOPCData(trans->opcData, count, clienthandles, values, quality, time, errors);
			trans->setCompleted();
		}
		return S_OK;
	}

	STDMETHODIMP OnWriteComplete(DWORD Transid, OPCHANDLE grphandle, HRESULT mastererr,
		DWORD count, OPCHANDLE* clienthandles, HRESULT* errors)
	{
		// TODO this is bad  - server could corrupt address - need to use look up table
		CTransaction* trans = m_CallbacksGroup.getServer().PeekTransaction(Transid);
		if (trans)
		{
			for (unsigned i = 0; i < count; i++) {
				// TODO this is bad  - server could corrupt address - need to use look up table
				COPCItem* item = m_CallbacksGroup.FindOpcItem(clienthandles[i]);
				if(item)
					trans->setItemError(item, errors[i]); // this records error state - may be good
			}
			trans->setCompleted();
		}
		return S_OK;
	}

	STDMETHODIMP OnCancelComplete(DWORD transid, OPCHANDLE grphandle) {
		printf("OnCancelComplete: Transid=%ld GrpHandle=%ld\n", transid, grphandle);
		return S_OK;
	}

	/**
	* make OPC item
	*/
	static OPCItemData* makeOPCDataItem(VARIANT& value, WORD quality, FILETIME& time, HRESULT error) {
		OPCItemData* data = NULL;
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
	void updateOPCData(COPCItem_DataMap& opcData, DWORD count, OPCHANDLE* clienthandles,
		VARIANT* values, WORD* quality, FILETIME* time, HRESULT* errors) {
		// see page 136 - returned arrays may be out of order
		for (unsigned i = 0; i < count; i++) {
			// TODO this is bad  - server could corrupt address - need to use look up table
			COPCItem* item = m_CallbacksGroup.FindOpcItem(clienthandles[i]);
			if (item)
			{
				OPCItemData* data = makeOPCDataItem(values[i], quality[i], time[i], errors[i]);
				COPCItem_DataMap::CPair* pair = opcData.Lookup(item);
				if (pair == NULL) {
					opcData.SetAt(item, data);
				}
				else {
					opcData.SetValueAt(pair, data);
				}
			}
		}
	}
};

COPCGroup::COPCGroup(const std::string& groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long& revisedUpdateRate_ms, float deadBand, COPCServer& server) :
	m_szGroupName(groupName),
	m_opcServer(server)
{
	std::wstring  wideName = CUtils::ANSIToUnicode(groupName);

	HRESULT result = m_opcServer.getServerInterface()->AddGroup(wideName.c_str(), active, reqUpdateRate_ms, 0, 0, &deadBand,
		0, &m_dwGroupHandle, &revisedUpdateRate_ms, IID_IOPCGroupStateMgt, (LPUNKNOWN*)& m_iStateManagement);
	if (FAILED(result))
	{
		throw OPCException("Failed to Add group", result);
	}

	result = m_iStateManagement->QueryInterface(IID_IOPCSyncIO, (void**)& m_iSychIO);
	if (FAILED(result)) {
		throw OPCException("Failed to get IID_IOPCSyncIO", result);
	}

	result = m_iStateManagement->QueryInterface(IID_IOPCAsyncIO2, (void**)& m_iAsych2IO);
	if (FAILED(result)) {
		throw OPCException("Failed to get IID_IOPCAsyncIO2", result);
	}

	result = m_iStateManagement->QueryInterface(IID_IOPCItemMgt, (void**)& m_iItemManagement);
	if (FAILED(result)) {
		throw OPCException("Failed to get IID_IOPCItemMgt", result);
	}
}

COPCGroup::~COPCGroup()
{
	RemoveAllItems();
	m_opcServer.getServerInterface()->RemoveGroup(m_dwGroupHandle, FALSE);
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
		m_mapItems.erase(it);//added by fangqing
	}
	m_mapItems.insert(std::pair<std::string, COPCItem*>(pItem->getName(), pItem));
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
		}
		m_mapItems.erase(it);
	}
}

void COPCGroup::RemoveAllItems()
{
	std::lock_guard<std::mutex> itemLock(m_ItemLock);

	std::map<std::string, COPCItem*>::iterator it;
#if 0
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
#else

	m_mapClientHandleCache.clear();
	it = m_mapItems.begin();
	for (; it != m_mapItems.end(); it++)
	{
		COPCItem* pItemFind = it->second;
		if (pItemFind)
		{
			delete pItemFind;
			pItemFind = NULL;
		}
	}
#endif
	m_mapItems.clear();
	
}

OPCHANDLE* COPCGroup::buildServerHandleList(std::vector<COPCItem*>& items) {
	OPCHANDLE* handles = new OPCHANDLE[items.size()];
	for (unsigned i = 0; i < items.size(); i++) {
		if (items[i] == NULL) {
			delete[]handles;
			throw OPCException("Item is NULL");
		}
		handles[i] = items[i]->getServerItemHandle();
	}
	return handles;
}

void COPCGroup::readSync(std::vector<COPCItem*>& items, COPCItem_DataMap& opcData, OPCDATASOURCE source) {
	OPCHANDLE* serverHandles = buildServerHandleList(items);
	HRESULT* itemResult;
	OPCITEMSTATE* itemState;
	DWORD noItems = (DWORD)items.size();

	HRESULT	result = m_iSychIO->Read(source, noItems, serverHandles, &itemState, &itemResult);
	if (FAILED(result)) {
		delete[]serverHandles;
		throw OPCException("Read failed", result);
	}

	for (unsigned i = 0; i < noItems; i++) {
		COPCItem* item = FindOpcItem(itemState[i].hClient);
		OPCItemData* data = CAsynchDataCallback::makeOPCDataItem(itemState[i].vDataValue, itemState[i].wQuality, itemState[i].ftTimeStamp, itemResult[i]);
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

CTransaction* COPCGroup::readAsync(std::vector<COPCItem*>& items, ITransactionComplete* transactionCB) {
	DWORD cancelID;
	HRESULT* individualResults;
	CTransaction* trans = new CTransaction(items, transactionCB);
	DWORD dwTransId = getServer().PushTransaction(trans);

	OPCHANDLE* serverHandles = buildServerHandleList(items);
	DWORD noItems = (DWORD)items.size();

	HRESULT result = m_iAsych2IO->Read(noItems, serverHandles, dwTransId, &cancelID, &individualResults);
	delete[] serverHandles;
	if (FAILED(result)) {
		delete trans;
		throw OPCException("Asynch Read failed", result);
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

CTransaction* COPCGroup::refresh(OPCDATASOURCE source, ITransactionComplete* transactionCB) {
	DWORD cancelID;
	CTransaction* trans = new CTransaction(m_items, transactionCB);
	DWORD dwTransId = getServer().PushTransaction(trans);

	HRESULT result = m_iAsych2IO->Refresh2(source, dwTransId, &cancelID);
	if (FAILED(result)) {
		delete trans;
		throw OPCException("refresh failed", result);
	}

	return trans;
}

DWORD COPCGroup::GetClientHandle()
{
	if (m_dwClientHandleId >= 0xFFFFFFFE)
		m_dwClientHandleId = 0;
	m_dwClientHandleId++;
	return m_dwClientHandleId;
}

DWORD COPCGroup::PushOpcItem(COPCItem* pItem)
{
	std::lock_guard<std::mutex> lk(m_ClientHandleLock);
	DWORD dwClientId = GetClientHandle();
	pItem->setClientItemHandle(dwClientId);
	m_mapClientHandleCache[dwClientId] = pItem;
	return dwClientId;
}

COPCItem* COPCGroup::FindOpcItem(DWORD dwClientHandle)
{
	std::lock_guard<std::mutex> lk(m_ClientHandleLock);
	COPCItem* pItem = nullptr;
	std::map<DWORD, COPCItem*>::iterator it = m_mapClientHandleCache.find(dwClientHandle);
	if (it != m_mapClientHandleCache.end())
	{
		pItem = it->second;
	}
	return pItem;
}

COPCItem* COPCGroup::PeekOpcItem(DWORD dwClientHandle)
{
	std::lock_guard<std::mutex> lk(m_ClientHandleLock);
	COPCItem* pItem = nullptr;
	std::map<DWORD, COPCItem*>::iterator it = m_mapClientHandleCache.find(dwClientHandle);
	if (it != m_mapClientHandleCache.end())
	{
		pItem = it->second;
		m_mapClientHandleCache.erase(dwClientHandle);
	}
	return pItem;
}

COPCItem* COPCGroup::addItem(std::string& itemName, bool active)
{
	std::vector<std::string> names;
	std::vector<COPCItem*> itemsCreated;
	std::vector<HRESULT> errors;
	names.push_back(itemName);
	if (addItems(names, itemsCreated, errors, active) != 0) {
		throw OPCException("Failed to add item", errors.front());
	}
	return itemsCreated[0];
}

bool COPCGroup::WriteOPCValue(const char* itemName, VARIANT& vtValue)
{
	bool bResult = false;
	std::lock_guard<std::mutex> itemLock(m_ItemLock);

	std::map<std::string, COPCItem*>::iterator it = m_mapItems.find(itemName);
	if (it != m_mapItems.end())
	{
		COPCItem* pItemFind = it->second;
		if (pItemFind)
		{
			CTransaction* pTrans = pItemFind->writeAsynch(vtValue, bResult);
			if (pTrans)
			{
				delete pTrans;
				pTrans = NULL;
			}
		}
	}
	return bResult;
}

int COPCGroup::addItems(std::vector<std::string>& itemName, std::vector<COPCItem*>& itemsCreated, std::vector<HRESULT>& errors, bool active) {
	itemsCreated.resize(itemName.size());
	errors.resize(itemName.size());
	OPCITEMDEF* itemDef = new OPCITEMDEF[itemName.size()];
	unsigned i = 0;
	DWORD dwClientId = 0;
	std::vector<CT2OLE*> tpm;
	for (; i < itemName.size(); i++) {
		itemsCreated[i] = new COPCItem(itemName[i], *this);
		dwClientId = PushOpcItem(itemsCreated[i]);
		USES_CONVERSION;
		tpm.push_back(new CT2OLE(itemName[i].c_str()));
		itemDef[i].szItemID = **(tpm.end() - 1);
		itemDef[i].szAccessPath = NULL;//wideName;
		itemDef[i].bActive = active;
//		itemDef[i].hClient = (DWORD)itemsCreated[i];
		itemDef[i].hClient = dwClientId;
		itemDef[i].dwBlobSize = 0;
		itemDef[i].pBlob = NULL;
		itemDef[i].vtRequestedDataType = VT_EMPTY;
	}

	HRESULT* itemResult;
	OPCITEMRESULT* itemDetails;
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
			PeekOpcItem(itemsCreated[i]->getClientItemHandle());
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

void COPCGroup::enableAsynch(IAsynchDataCallback& handler) {
	if (!m_asynchDataCallBackHandler == false) {
		throw OPCException("Asynch already enabled");
	}

	ATL::CComPtr<IConnectionPointContainer> iConnectionPointContainer = 0;
	HRESULT result = m_iStateManagement->QueryInterface(IID_IConnectionPointContainer, (void**)& iConnectionPointContainer);
	if (FAILED(result))
	{
		throw OPCException("Could not get IID_IConnectionPointContainer", result);
	}


	result = iConnectionPointContainer->FindConnectionPoint(IID_IOPCDataCallback, &m_iAsynchDataCallbackConnectionPoint);
	if (FAILED(result))
	{
		throw OPCException("Could not get IID_IOPCDataCallback", result);
	}


	m_asynchDataCallBackHandler = new CAsynchDataCallback(*this);
	result = m_iAsynchDataCallbackConnectionPoint->Advise(m_asynchDataCallBackHandler, &m_dwCallbackHandle);
	if (FAILED(result))
	{
		m_iAsynchDataCallbackConnectionPoint = NULL;
		m_asynchDataCallBackHandler = NULL;
		throw OPCException("Failed to set DataCallbackConnectionPoint", result);
	}

	m_pUserAsynchCBHandler = &handler;
}

void COPCGroup::setState(DWORD reqUpdateRate_ms, DWORD& returnedUpdateRate_ms, float deadBand, BOOL active) {
	HRESULT result = m_iStateManagement->SetState(&reqUpdateRate_ms, &returnedUpdateRate_ms, &active, 0, &deadBand, 0, 0);
	if (FAILED(result))
	{
		throw OPCException("Failed to set group state", result);
	}
}

void COPCGroup::disableAsynch() {
	if (m_asynchDataCallBackHandler) {
		m_iAsynchDataCallbackConnectionPoint->Unadvise(m_dwCallbackHandle);
		m_iAsynchDataCallbackConnectionPoint = NULL;
		m_asynchDataCallBackHandler = NULL;// WE DO NOT DELETE callbackHandler, let the COM ref counting take care of that
	}
	if (m_pUserAsynchCBHandler)
	{
		delete m_pUserAsynchCBHandler;
		m_pUserAsynchCBHandler = NULL;
	}
}