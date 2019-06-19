#pragma once

#include "OPCException.h"
#include "opcstructs.h"
#include <vector>

class OPCSDKEXPORT OpcClientSDK
{
public:
	//初始化环境
	virtual bool Initialize(OPCOLEInitMode mode = APARTMENTTHREADED) = 0;
	//卸载环境
	virtual bool Uninitialize() = 0;
	//获取主机中OPC2.0服务列表
	virtual bool GetCLSIDList(const char* svrAddr, std::vector<std::string>& list, OPCException* ex = NULL) = 0;
	//连接OPC服务
	virtual bool ConnectServer(const char* svrAddr, const char* progid, OPCException* ex = NULL) = 0;
	//断开OPC服务
	virtual bool DisConnectServer(const char* svrAddr, const char* progid, OPCException* ex = NULL) = 0;
	//获取服务状态
	virtual bool GetServerStatus(ServerStatus& status, OPCException* ex = NULL) = 0;
	//获取点表
	virtual bool GetItemsList(std::vector<std::string>& lstItems, OPCException* ex = NULL) = 0;
	//添加组
	virtual bool AddGroup(const char* groupName, unsigned long& refreshRate, OPCException* ex = NULL) = 0;
	//删除组
	virtual bool RemoveGroup(const char* groupName, OPCException* ex = NULL) = 0;
	//添加读取项
	virtual bool AddItems(const char* groupName, std::vector<std::string> lstAdded, std::vector<long>& errors, std::vector<VARTYPE>& dataTypes, OPCException* ex = NULL) = 0;
	//删除读取项
	virtual bool RemoveItems(const char* groupName, std::vector<std::string> lstDel, OPCException* ex = NULL) = 0;
	//写OPC值异步
	virtual bool WriteOPCValue(const char* groupName, const char* itemName, VARIANT& var, OPCException* ex = NULL) = 0;
	//值回调
	virtual void SetValueReport(OPCValueEventCallBack pFun, void* pUser) = 0;
public:
	static OpcClientSDK* CreateOPCSDK();
	static void DestroyOPCSDK(OpcClientSDK* pSdk);
};

