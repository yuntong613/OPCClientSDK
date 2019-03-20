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

#if !defined(AFX_OPCSERVER_H__AD6316C0_37B3_4DEC_8378_EE03CC3AEED8__INCLUDED_)
#define AFX_OPCSERVER_H__AD6316C0_37B3_4DEC_8378_EE03CC3AEED8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COPCGroup;

#include "opcda.h"
#include <vector>
#include "opcstructs.h"
#include <mutex>
#include <map>

/**
* Local representation of a local or remote OPC server. Wrapper for the COM interfaces to the server.
*/
class COPCServer
{
private:
	/**
	* IUnknown interface to the OPC server
	*/
	ATL::CComPtr<IOPCServer> iOpcServer;

	/**
	* Interface to the OPC server namespace
	*/
	ATL::CComPtr<IOPCBrowseServerAddressSpace> iOpcNamespace;

	/**
	* interface to the properties maintained for each item in the server namespace
	*/
	ATL::CComPtr<IOPCItemProperties> iOpcProperties;

	friend class COPCGroup;
	/**
	* Used by group object.
	*/
	ATL::CComPtr<IOPCServer> &getServerInterface() {
		return iOpcServer;
	}

	friend class COPCItem;
	ATL::CComPtr<IOPCItemProperties> &getPropertiesInterface() {
		return iOpcProperties;
	}

	/**
	* Name of the server
	*/
	const std::string name;

	std::mutex m_groupLock;
	std::map<std::string, COPCGroup*> m_mapGroups;
public:

	/**
	* Make OPC server.
	* @param opcServerInterface passed form the OPCHost
	*/
	COPCServer(ATL::CComPtr<IOPCServer> &opcServerInterface,std::string svrname);

	virtual ~COPCServer();

	void AddGroupToMap(COPCGroup* pGroup);

	void RemoveGroupFromMap(const char* groupName);

	void RemoveAllGroups(COPCGroup* pGroup);

	bool WriteOPCValue(const char* groupName, const char* itemName, VARIANT& vtValue);
	//添加读取项
	bool AddItems(const char* groupName, std::vector<std::string> lstAdded);

	//删除读取项
	bool RemoveItems(const char* groupName, std::vector<std::string> lstDel);
	//断开连接
	void ShutdownRequest(LPCTSTR lpszReason);
	/**
	* Browse the OPC servers namespace.
	* This is currently done FLAT mode
	* TODO implement browsing of structured namespace
	*/
	void getItemNames(std::vector<std::string> & opcItemNames);

	/**
	* Get an OPC group. Caller owns
	*/
	COPCGroup *makeGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand);

	/**
	* get the current status of the server.
	*/
	void getStatus(ServerStatus &status);

	const std::string & getName() const {
		return name;
	}
};

#endif // !defined(AFX_OPCSERVER_H__AD6316C0_37B3_4DEC_8378_EE03CC3AEED8__INCLUDED_)
