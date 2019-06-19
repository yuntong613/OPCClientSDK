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
#include "OPCException.h"
#include "OPCGroup.h"
#include "OPCItem.h"

COPCServer::COPCServer(ATL::CComPtr<IOPCServer> &opcServerInterface, std::string svrname) :name(svrname)
{
	iOpcServer = opcServerInterface;
	HRESULT res = opcServerInterface->QueryInterface(IID_IOPCBrowseServerAddressSpace, (void**)&iOpcNamespace);
	if (FAILED(res)) {
		iOpcNamespace = NULL;
		throw OPCException("Failed to obtain IID_IOPCBrowseServerAddressSpace interface", res);
	}

	res = opcServerInterface->QueryInterface(IID_IOPCItemProperties, (void**)&iOpcProperties);
	if (FAILED(res)) {
		iOpcProperties = NULL;
		throw OPCException("Failed to obtain IID_IOPCItemProperties interface", res);
	}
}



COPCServer::~COPCServer()
{
	RemoveAllGroups();
}

bool COPCServer::exist_group(const char* pgroup_name)
{
	bool exist = false;
	std::lock_guard<std::mutex> lk(m_groupLock);

	std::map<std::string, COPCGroup*>::iterator it = m_mapGroups.find(pgroup_name);
	if (it != m_mapGroups.end())
	{
		exist = true;
	}
	return exist;
}

void COPCServer::AddGroupToMap(COPCGroup* pGroup)
{
	std::lock_guard<std::mutex> lk(m_groupLock);
	m_mapGroups.insert(std::pair<std::string, COPCGroup*>(pGroup->getName(), pGroup));
}

void COPCServer::RemoveGroupFromMap(const char* groupName)
{
	std::lock_guard<std::mutex> lk(m_groupLock);

	std::map<std::string, COPCGroup*>::iterator it = m_mapGroups.find(groupName);
	if (it != m_mapGroups.end())
	{
		COPCGroup* pGroupFind = it->second;
		if (pGroupFind)
		{
			pGroupFind->disableAsynch();
			delete pGroupFind;
			pGroupFind = NULL;
			m_mapGroups.erase(it);
		}
	}
}

void COPCServer::RemoveAllGroups()
{
	std::lock_guard<std::mutex> lk(m_groupLock);

	std::map<std::string, COPCGroup*>::iterator it;
	while (!m_mapGroups.empty())
	{
		it = m_mapGroups.begin();
		COPCGroup* pGroupFind = it->second;
		if (pGroupFind)
		{
			pGroupFind->disableAsynch();
			delete pGroupFind;
			pGroupFind = NULL;
			m_mapGroups.erase(it);
		}
	}
}

bool COPCServer::WriteOPCValue(const char* groupName, const char* itemName, VARIANT& vtValue)
{
	std::lock_guard<std::mutex> lk(m_groupLock);

	std::map<std::string, COPCGroup*>::iterator it = m_mapGroups.find(groupName);
	if (it != m_mapGroups.end())
	{
		COPCGroup* pGroupFind = it->second;
		if (pGroupFind)
		{
			return pGroupFind->WriteOPCValue(itemName, vtValue);
		}
	}
	return false;
}

bool COPCServer::AddItems(const char* groupName, std::vector<std::string> lstAdded, std::vector<long>& errors, std::vector<VARTYPE>& dataTypes)
{
	std::lock_guard<std::mutex> lk(m_groupLock);
	std::map<std::string, COPCGroup*>::iterator it = m_mapGroups.find(groupName);
	if (it != m_mapGroups.end())
	{
		COPCGroup* pGroup = it->second;
		if (pGroup)
		{
			dataTypes.resize(lstAdded.size());
			std::vector<COPCItem*> items;
			int nErrorCount = pGroup->addItems(lstAdded, items, errors, true);
			for (size_t i = 0; i < items.size(); i++)
			{
				dataTypes[i] = VT_UNKNOWN;
				COPCItem* pItem = items[i];
				if (pItem)
				{
					dataTypes[i] = pItem->getDataType();
					pGroup->AddItemToMap(pItem);
				}
			}
			return true;
		}
	}
	return false;
}

bool COPCServer::RemoveItems(const char* groupName, std::vector<std::string> lstDel)
{
	std::lock_guard<std::mutex> lk(m_groupLock);
	std::vector<std::string> tmp = lstDel;

	std::map<std::string, COPCGroup*>::iterator it = m_mapGroups.find(groupName);
	if (it != m_mapGroups.end())
	{
		COPCGroup* pGroup = it->second;
		if (pGroup)
		{
			for (size_t i=0;i<lstDel.size();i++)
			{
				std::string itemName = lstDel[i];
				pGroup->RemoveItemFromMap(itemName.c_str());
			}		
			return true;
		}
	}
	return false;
}

void COPCServer::ShutdownRequest(LPCTSTR lpszReason)
{
	
}

COPCGroup *COPCServer::makeGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand) {
	return new COPCGroup(groupName, active, reqUpdateRate_ms, revisedUpdateRate_ms, deadBand, *this);
}


void COPCServer::getItemNames(std::vector<std::string> & opcItemNames) {
	if (!iOpcNamespace) return;

	OPCNAMESPACETYPE nameSpaceType;
	HRESULT result = iOpcNamespace->QueryOrganization(&nameSpaceType);

	int v = 0;
	WCHAR emptyString[] = { 0 };
	//result = iOpcNamespace->ChangeBrowsePosition(OPC_BROWSE_TO,emptyString);

	ATL::CComPtr<IEnumString> iEnum;
	result = iOpcNamespace->BrowseOPCItemIDs(OPC_FLAT, emptyString, VT_EMPTY, 0, (&iEnum));
	if (FAILED(result)) {
		return;
	}

	WCHAR * str;
	ULONG strSize;
	while ((result = iEnum->Next(1, &str, &strSize)) == S_OK)
	{
		WCHAR * fullName;
		result = iOpcNamespace->GetItemID(str, &fullName);
		if (SUCCEEDED(result)) {
			std::string cStr = CUtils::UnicodeToANSI(fullName);
			//char * cStr = OLE2T(str);
			//printf("Adding %s\n", cStr);
			opcItemNames.push_back(cStr);
			CoTaskMemFree(fullName);
		}
		CoTaskMemFree(str);
	}
}


void COPCServer::getStatus(ServerStatus &status) {
	OPCSERVERSTATUS *serverStatus;
	HRESULT result = iOpcServer->GetStatus(&serverStatus);
	if (FAILED(result)) {
		throw OPCException("Failed to get status");
	}

	status.ftStartTime = serverStatus->ftStartTime;
	status.ftCurrentTime = serverStatus->ftCurrentTime;
	status.ftLastUpdateTime = serverStatus->ftLastUpdateTime;
	status.dwServerState = serverStatus->dwServerState;
	status.dwGroupCount = serverStatus->dwGroupCount;
	status.dwBandWidth = serverStatus->dwBandWidth;
	status.wMajorVersion = serverStatus->wMajorVersion;
	status.wMinorVersion = serverStatus->wMinorVersion;
	status.wBuildNumber = serverStatus->wBuildNumber;
	if (serverStatus->szVendorInfo != NULL) {
		status.vendorInfo = CUtils::UnicodeToANSI(serverStatus->szVendorInfo);
		CoTaskMemFree(serverStatus->szVendorInfo);
	}
	CoTaskMemFree(serverStatus);
}