#pragma once

#include "OpcClientSDK.h"
#include "opcstructs.h"
#include "MyDataCallBack.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

class COPCHost;
class COPCGroup;
class COPCServer;

class OpcClientSDKImp : public OpcClientSDK
{
public:
	OpcClientSDKImp();
	~OpcClientSDKImp();
	//初始化COM环境
	virtual bool Initialize(OPCOLEInitMode mode = APARTMENTTHREADED);
	//卸载COM环境
	virtual bool Uninitialize();
	//获取主机中OPC2.0服务列表
	virtual bool GetCLSIDList(const char* svrAddr, std::vector<std::string>& list, OPCException* ex = NULL);
	//连接OPC服务
	virtual bool ConnectServer(const char* svrAddr, const char* progid, OPCException* ex=NULL);
	//断开OPC服务
	virtual bool DisConnectServer(const char* svrAddr, const char* progid, OPCException* ex=NULL);
	//获取服务状态
	virtual bool GetServerStatus(ServerStatus& status, OPCException* ex = NULL);
	//获取点表
	virtual bool GetItemsList(std::vector<std::string>& lstItems, OPCException* ex= NULL);
	//添加组
	virtual bool AddGroup(const char* groupName, unsigned long& refreshRate, OPCException* ex= NULL);
	//删除组
	virtual bool RemoveGroup(const char* groupName, OPCException* ex= NULL);
	//添加读取项
	virtual bool AddItems(const char* groupName, std::vector<std::string> lstAdded, OPCException* ex= NULL);
	//删除读取项
	virtual bool RemoveItems(const char* groupName, std::vector<std::string> lstDel, OPCException* ex= NULL);

	virtual bool WriteOPCValue(const char* groupName, const char* itemName, VARIANT& var, OPCException* ex= NULL);

	virtual void SetValueReport(OPCValueEventCallBack pFun, void* pUser);


protected:
	COPCHost * makeHost(const std::string &hostName);
protected:
	COPCHost* m_pHost;
	COPCServer* m_pServer;
	OPCValueEventCallBack m_ValueReport;
	void* m_pUser;

	MyDataCallBack* m_pCallBack;
};
