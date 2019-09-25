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

#if !defined(AFX_OPCGROUP_H__BE6D983E_3D18_4952_A2B3_84A9FCDFC5CE__INCLUDED_)
#define AFX_OPCGROUP_H__BE6D983E_3D18_4952_A2B3_84A9FCDFC5CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Transaction.h"
#include "IAsynchDataCallback.h"
#include <mutex>
#include <map>
/**
* Forward decl.
*/
class COPCItem;
class COPCServer;

/**
* used internally to implement the asynch callback
*/
class CAsynchDataCallback;

/**
* Client sided abstraction of an OPC group, wrapping the COM interfaces to the group within the OPC server.
*/
class COPCGroup
{
private:
	ATL::CComPtr<IOPCGroupStateMgt>	m_iStateManagement;
	ATL::CComPtr<IOPCSyncIO>		m_iSychIO;
	ATL::CComPtr<IOPCAsyncIO2>		m_iAsych2IO;
	ATL::CComPtr<IOPCItemMgt>		m_iItemManagement;

	/**
	* Used to keep track of the connection point for the
	* AsynchDataCallback
	*/
	ATL::CComPtr<IConnectionPoint> m_iAsynchDataCallbackConnectionPoint;

	/**
	* handle given the group by the server
	*/
	DWORD m_dwGroupHandle;

	/**
	* The server this group belongs to
	*/
	COPCServer& m_opcServer;

	/**
	* Callback for asynch data at the group level
	*/
	ATL::CComPtr<CAsynchDataCallback> m_asynchDataCallBackHandler;

	/**
	* list of OPC items associated with this goup. Not owned (at the moment!)
	*/
	std::vector<COPCItem*> m_items;

	/**
	* Name of the group
	*/
	const std::string m_szGroupName;

	/**
	* Handle given to callback by server.
	*/
	DWORD m_dwCallbackHandle;

	/**
	* Users hander to handle asynch data
	* NOT OWNED.
	*/
	IAsynchDataCallback* m_pUserAsynchCBHandler;

	/**
	* Caller owns returned array
	*/
	OPCHANDLE* buildServerHandleList(std::vector<COPCItem*>& items);

	std::mutex m_ItemLock;
	std::map<std::string, COPCItem*> m_mapItems;
public:
	COPCGroup(const std::string& groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long& revisedUpdateRate_ms, float deadBand, COPCServer& server);

	virtual ~COPCGroup();

	void AddItemToMap(COPCItem* pItem);

	void RemoveItemFromMap(const char* itemName);

	void RemoveAllItems();

	COPCItem* addItem(std::string& itemName, bool active);

	bool WriteOPCValue(const char* itemName, VARIANT& vtValue);
	/**
	* returns the number of failed item creates
	* itemsCreated[x] will be null if could not create and will contain error code in corresponding error entry
	*/
	int addItems(std::vector<std::string>& itemName, std::vector<COPCItem*>& itemsCreated, std::vector<HRESULT>& errors, bool active);

	/**
	* enable Asynch IO
	*/
	void enableAsynch(IAsynchDataCallback& handler);


	/**
	* disable Asych IO
	*/
	void disableAsynch();


	/**
	* set the group state values.
	*/
	void setState(DWORD reqUpdateRate_ms, DWORD& returnedUpdateRate_ms, float deadBand, BOOL active);



	/**
	* Read set of OPC items synchronously.
	*/
	void readSync(std::vector<COPCItem*>& items, COPCItem_DataMap& opcData, OPCDATASOURCE source);


	/**
	* Read a defined group of OPC item asynchronously
	*/
	CTransaction* readAsync(std::vector<COPCItem*>& items, ITransactionComplete* transactionCB = NULL);


	/**
	* Refresh is an asysnch operation.
	* retreives all active items in the group, which will be stored in the transaction object
	* Transaction object is owned by caller.
	* If group asynch is disabled then this call will not work
	*/
	CTransaction* refresh(OPCDATASOURCE source, ITransactionComplete* transactionCB = NULL);



	ATL::CComPtr<IOPCSyncIO>& getSychIOInterface() {
		return m_iSychIO;
	}


	ATL::CComPtr<IOPCAsyncIO2>& getAsych2IOInterface() {
		return m_iAsych2IO;
	}


	ATL::CComPtr<IOPCItemMgt>& getItemManagementInterface() {
		return m_iItemManagement;
	}

	const std::string& getName() const {
		return m_szGroupName;
	}

	IAsynchDataCallback* getUsrAsynchHandler() {
		return m_pUserAsynchCBHandler;
	}

	/**
	* returns reaference to the OPC server that this group belongs to.
	*/
	COPCServer& getServer() {
		return m_opcServer;
	}

public:
		std::mutex m_ClientHandleLock;
		DWORD GetClientHandle();
		DWORD PushOpcItem(COPCItem* pItem);
		COPCItem* FindOpcItem(DWORD dwClientHandle);

		COPCItem* PeekOpcItem(DWORD dwClientHandle);
		std::map<DWORD, COPCItem*> m_mapClientHandleCache;
protected:
	DWORD m_dwClientHandleId;
};

#endif // !defined(AFX_OPCGROUP_H__BE6D983E_3D18_4952_A2B3_84A9FCDFC5CE__INCLUDED_)
