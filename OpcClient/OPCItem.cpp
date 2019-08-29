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
#include "OPCItem.h"
#include "OPCGroup.h"
#include "OPCException.h"

COPCItem::COPCItem(std::string& itemName, COPCGroup& g)
	:name(itemName), group(g)
{
}



COPCItem::~COPCItem()
{
	HRESULT* itemResult = NULL;
	group.getItemManagementInterface()->RemoveItems(1, &serversItemHandle, &itemResult);
	CoTaskMemFree(itemResult);
}

void COPCItem::setOPCParams(OPCHANDLE handle, VARTYPE type, DWORD dwAccess) {
	serversItemHandle = handle;
	vtCanonicalDataType = type;
	dwAccessRights = dwAccess;
}

void COPCItem::writeSync(VARIANT& data) {
	HRESULT* itemWriteErrors;
	HRESULT result = group.getSychIOInterface()->Write(1, &serversItemHandle, &data, &itemWriteErrors);
	if (FAILED(result))
	{
		throw OPCException("write failed", result);
	}

	if (FAILED(itemWriteErrors[0])) {
		HRESULT res = itemWriteErrors[0];
		CoTaskMemFree(itemWriteErrors);
		throw OPCException("write failed", res);
	}

	CoTaskMemFree(itemWriteErrors);
}

void COPCItem::readSync(OPCItemData& data, OPCDATASOURCE source) {
	std::vector<COPCItem*> items;
	items.push_back(this);
	COPCItem_DataMap opcData;
	group.readSync(items, opcData, source);

	COPCItem_DataMap::CPair* pos = opcData.Lookup(this);
	if (pos) {
		OPCItemData* readData = opcData.GetValueAt(pos);
		if (readData && !FAILED(readData->error)) {
			data = *readData;
			return;
		}
	}

	throw OPCException("Read failed");
}

CTransaction* COPCItem::readAsynch(ITransactionComplete* transactionCB) {
	std::vector<COPCItem*> items;
	items.push_back(this);
	return group.readAsync(items, transactionCB);
}

CTransaction* COPCItem::writeAsynch(VARIANT& data, bool& bResult, ITransactionComplete* transactionCB /*= NULL*/)
{
	bResult = true;
	DWORD cancelID;
	HRESULT* individualResults;
	std::vector<COPCItem*> items;
	items.push_back(this);
	CTransaction* trans = new CTransaction(items, transactionCB);

	HRESULT result = group.getAsych2IOInterface()->Write(1, &serversItemHandle, &data, (DWORD)trans, &cancelID, &individualResults);

	if (FAILED(result)) {
		delete trans;
		throw OPCException("Asynch Write failed", result);
	}

	trans->setCancelId(cancelID);
	if (FAILED(individualResults[0])) {
		trans->setItemError(this, individualResults[0]);
		trans->setCompleted(); // if all items return error then no callback will occur. p 104
		bResult = false;
	}
	CoTaskMemFree(individualResults);

	return trans;
}

void COPCItem::getSupportedProperties(std::vector<CPropertyDescription>& desc) {
	DWORD noProperties = 0;
	DWORD* pPropertyIDs;
	LPWSTR* pDescriptions;
	VARTYPE* pvtDataTypes;

	wstring szItemID = CUtils::ANSIToUnicode(name);

	HRESULT res = group.getServer().getPropertiesInterface()->QueryAvailableProperties((WCHAR*)szItemID.c_str(), &noProperties, &pPropertyIDs, &pDescriptions, &pvtDataTypes);
	if (FAILED(res)) {
		throw OPCException("Failed to restrieve properties", res);
	}

	std::string tmp;
	for (unsigned i = 0; i < noProperties; i++) {
		tmp = CW2A(pDescriptions[i]);
		desc.push_back(CPropertyDescription(pPropertyIDs[i], std::string(tmp), pvtDataTypes[i]));
	}

	CoTaskMemFree(pPropertyIDs);
	CoTaskMemFree(pDescriptions);
	CoTaskMemFree(pvtDataTypes);
}


void COPCItem::getProperties(const std::vector<CPropertyDescription>& propsToRead, ATL::CAutoPtrArray<SPropertyValue>& propsRead) {
	unsigned noProperties = (DWORD)propsToRead.size();
	VARIANT* pValues = NULL;
	HRESULT* pErrors = NULL;
	DWORD* pPropertyIDs = new DWORD[noProperties];
	for (unsigned i = 0; i < noProperties; i++) {
		pPropertyIDs[i] = propsToRead[i].id;
	}
	propsRead.RemoveAll();
	propsRead.SetCount(noProperties);

	wstring szItemID = CUtils::ANSIToUnicode(name);
	HRESULT res = group.getServer().getPropertiesInterface()->GetItemProperties((WCHAR*)szItemID.c_str(), noProperties, pPropertyIDs, &pValues, &pErrors);
	delete[]pPropertyIDs;
	if (FAILED(res)) {
		throw OPCException("Failed to restrieve property values", res);
	}

	for (unsigned i = 0; i < noProperties; i++) {
		CAutoPtr<SPropertyValue> v;
		if (!FAILED(pErrors[i])) {
			v.Attach(new SPropertyValue(propsToRead[i], pValues[i]));
		}
		propsRead[i] = v;
	}

	for (unsigned i = 0; i < noProperties; i++) {
		VariantClear(&(pValues[i]));
	}

	CoTaskMemFree(pErrors);
	CoTaskMemFree(pValues);
}