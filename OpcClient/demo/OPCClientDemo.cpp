 // OPCClientDemo.cpp : Defines the entry point for the console application.
//

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
#include "..\stdafx.h"
#include <stdio.h>
#include "..\opcda.h"
#include "..\OPCHost.h"
#include "..\OPCServer.h"
#include "..\OPCGroup.h"
#include "..\OPCItem.h"
#include <sys\timeb.h>


/**
* Code demonstrates
* 1) Browsing local servers.
* 2) Connection to local server.
* 3) Browsing local server namespace
* 4) Creation of OPC item and group
* 5) synch and asynch read of OPC item.
* 6) synch and asynch write of OPC item.
* 7) The receipt of changes to items within a group (subscribe)
* 8) group refresh.
* 9) Synch read of multiple OPC items.
*/

std::string GetOpcValue(VARIANT vtVar)
{
	char tmp[256] = { 0 };
	std::string strValue;
	switch (vtVar.vt)
	{
	case VT_BOOL:
		sprintf(tmp, "%d", vtVar.boolVal ? 1 : 0);
		break;
	case VT_UI1:
		sprintf(tmp, "%u", vtVar.bVal);
		break;
	case VT_I1:
		sprintf(tmp, "%d", vtVar.cVal);
		break;
	case VT_UI2:
		sprintf(tmp,"%u", vtVar.uiVal);
		break;
	case VT_I2:
		sprintf(tmp,"%d", vtVar.iVal);
		break;
	case VT_UI4:
		sprintf(tmp, "%u", vtVar.ulVal);
		break;
	case VT_I4:
		sprintf(tmp,"%d", vtVar.lVal);
		break;
	case VT_R4:
		sprintf(tmp, "%.2f", vtVar.fltVal);
		break;
	case VT_R8:
		sprintf(tmp, "%.2f", vtVar.dblVal);
		break;
	case VT_BSTR:
		sprintf(tmp, "%s", vtVar.bstrVal);
		break;
	default:
		break;
	}
	strValue = tmp;
	return strValue;
}
/**
* Handle asynch data coming from changes in the OPC group
*/
class CMyCallback:public IAsynchDataCallback{
	public:
		void OnDataChange(COPCGroup & group, CAtlMap<COPCItem *, OPCItemData *> & changes){
			printf("Group %s, item changes\n", group.getName().c_str());

			std::string svr = group.getServer().getName();


			POSITION pos = changes.GetStartPosition();
			while (pos != NULL){
				CAtlMap<COPCItem *, OPCItemData *>::CPair * item = changes.GetNext(pos);
				COPCItem* pItem = item->m_key;
				OPCItemData* pData = item->m_value;

				printf("-----> %s  %s\n", pItem->getName().c_str(),GetOpcValue(pData->vDataValue).c_str());
			}
		}
		
};

/**
*	Handle completion of asynch operation
*/
class CTransComplete:public ITransactionComplete{
	std::string completionMessage;
public:
	CTransComplete(){
		completionMessage = "Asynch operation is completed";
	};

	void complete(CTransaction &transaction){
		printf("%s\n",completionMessage.c_str());
	}

	void setCompletionMessage(const std::string & message){
		completionMessage = message;
	}
};




//---------------------------------------------------------
// main



#define MESSAGEPUMPUNTIL(x)	while(!x){{MSG msg;while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE)){TranslateMessage(&msg);DispatchMessage(&msg);}Sleep(1);}}

void main(void)
{
	COPCClient::init();

	std::string hostName = "";
	COPCHost *host = COPCClient::makeHost(hostName);
	
	//List servers
	std::vector<std::string> localServerList;
	host->getListOfDAServers(IID_CATID_OPCDAServer20, localServerList);
	unsigned i = 0;
	printf("\nServer ID List:\n");
	for (; i < localServerList.size(); i++){
		printf("%d. %s\n", i,localServerList[i].c_str());
	}

	// connect to OPC
	printf("\nSelect a Server ID:\n");
	int server_id;
	scanf("%d",&server_id);
	std::string serverName = localServerList[server_id];
	COPCServer *opcServer = host->connectDAServer(serverName);

	// Check status
	ServerStatus status;
	opcServer->getStatus(status);
	printf("Server state is %ld\n", status.dwServerState);

	// browse server
	std::vector<std::string> opcItemNames;
	opcServer->getItemNames(opcItemNames);
	printf("Got %d names\n", opcItemNames.size());
	for (i = 0; i < opcItemNames.size(); i++){
		printf("%s\n",opcItemNames[i].c_str());
	}

	// make group
	unsigned long refreshRate;
	COPCGroup *group = opcServer->makeGroup("Group1",true,1000,refreshRate,0.0);
	std::vector<COPCItem *> items;

	// make  a single item
	std::string changeChanNameName = opcItemNames[5];
	COPCItem * readWritableItem = group->addItem(changeChanNameName, true);

	// make several items
	std::vector<std::string> itemNames;
	std::vector<COPCItem *>itemsCreated;
	std::vector<HRESULT> errors;

// 	itemNames.push_back(opcItemNames[15]);
// 	itemNames.push_back(opcItemNames[16]);	
	if (group->addItems(opcItemNames, itemsCreated,errors,true) != 0){
		printf("Item create failed\n");
	}

// 	// get properties
// 	std::vector<CPropertyDescription> propDesc;
// 	readWritableItem->getSupportedProperties(propDesc);
// 	printf("Supported properties for %s\n", readWritableItem->getName().c_str());
// 	for (i = 0; i < propDesc.size(); i++){
// 		printf("%d id = %u, description = %s, type = %d\n", i, propDesc[i].id,propDesc[i].desc.c_str(),propDesc[i].type);
// 	}
// 
// 	CAutoPtrArray<SPropertyValue> propVals;
// 	readWritableItem->getProperties(propDesc, propVals);
// 	for (i = 0; i < propDesc.size(); i++){
// 		if (propVals[i] == NULL){
// 			printf("Failed to get property %u\n", propDesc[i].id);
// 		}else{
// 			printf("Property %u=", propDesc[i].id);
// 			switch (propDesc[i].type){
// 				case VT_R4:printf("%f\n", propVals[i]->value.fltVal); break;
// 				case VT_I4:printf("%d\n", propVals[i]->value.iVal); break;
// 				case VT_I2:printf("%d\n", propVals[i]->value.iVal); break;
// 				case VT_I1:{int v = propVals[i]->value.bVal;printf("%d\n", v);} break;
// 				default:printf("\n");break;
// 			}
// 		}
// 	}


	// SYNCH READ of item
	OPCItemData data;
	readWritableItem->readSync(data,OPC_DS_DEVICE);
	printf("Synch read quality %d value %d\n", data.wQuality, data.vDataValue.iVal);

	// SYNCH read on Group
	COPCItem_DataMap opcData;
	group->readSync(itemsCreated,opcData, OPC_DS_DEVICE);

	// Enable asynch - must be done before any asynch call will work
	CMyCallback usrCallBack;
	group->enableAsynch(usrCallBack);
	
	// ASYNCH OPC item READ
	CTransComplete complete;
	complete.setCompletionMessage("*******Asynch read completion handler has been invoked (OPC item)");
	CTransaction* t = readWritableItem->readAsynch(&complete);
/*	MESSAGEPUMPUNTIL(t->isCompeleted())*/
	
// 	const OPCItemData * asychData = t->getItemValue(readWritableItem); // not owned
// 	if (!FAILED(asychData->error)){
// 		printf("Asynch read quality %d value %d\n", asychData->wQuality, asychData->vDataValue.iVal);
// 	}
	delete t;

	// Aysnch read opc items from a group
	complete.setCompletionMessage("*******Asynch read completion handler has been invoked (OPC GROUP)");
	t = group->readAsync(itemsCreated, &complete);
//	MESSAGEPUMPUNTIL(t->isCompeleted())
	delete t;
	

	// SYNCH write
	VARIANT var;
	var.vt = VT_I2;
	var.iVal = 99;
	readWritableItem->writeSync(var);


// 	// ASYNCH write
// 	var.vt = VT_I2;
// 	var.iVal = 32;
// 	complete.setCompletionMessage("*******Asynch write completion handler has been invoked");
// 	t = readWritableItem->writeAsynch(var, &complete);
// 	MESSAGEPUMPUNTIL(t->isCompeleted())
// 
// 	const OPCItemData *asychData = t->getItemValue(readWritableItem); // not owned
// 	if (!FAILED(asychData->error)){
// 		printf("Asynch write comleted OK\n");
// 	}else{
// 		printf("Asynch write failed\n");
// 	}
// 	delete t;
	

// 	// Group refresh (asynch operation) - pass results to CMyCallback as well
// 	complete.setCompletionMessage("*******Refresh completion handler has been invoked");
// 	t = group->refresh(OPC_DS_DEVICE, /*true,*/ &complete);
// 	MESSAGEPUMPUNTIL(t->isCompeleted())
// 
// 	// readWritableItem is member of group - look for this and use it as a guide to see if operation succeeded.
// 	asychData = t->getItemValue(readWritableItem); 
// 	if (!FAILED(asychData->error)){
// 		printf("refresh compeleted OK\n");
// 	}else{
// 		printf("refresh failed\n");
// 	}
// 	delete t;


	// just loop - changes to Items within a group are picked up here 
	MESSAGEPUMPUNTIL(false)
}