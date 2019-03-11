#pragma once

#include "opcstructs.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

class COPCHost;
class COPCGroup;
class COPCServer;

class OpcClientSDK
{
public:
	OpcClientSDK();
	~OpcClientSDK();
	//初始化COM环境
	bool InitOPCSdk(OPCOLEInitMode mode = APARTMENTTHREADED);
	//卸载COM环境
	bool DestroyOPCSdk();
	//获取主机中OPC2.0服务列表
	void GetCLSIDList(const char* svrAddr, std::vector<std::string>& list);
	//连接OPC服务
	bool ConnectServer(const char* svrAddr, const char* progid);
	//断开OPC服务
	bool DisConnectServer(const char* svrAddr, const char* progid);
	//获取服务状态
	bool GetServerStatus(ServerStatus& status);
	//获取点表
	bool GetItemsList(std::vector<std::string>& lstItems);
	//添加组
	bool AddGroup(const char* name, unsigned long& refreshRate);
	//删除组
	bool RemoveGroup(const char* name);
	//添加读取项
	bool AddItems(const char* group, std::vector<std::string> lstAdded);
	//删除读取项
	bool RemoveItems(const char* group, std::vector<std::string> lstDel);

protected:
	COPCHost * makeHost(const std::string &hostName);
protected:
	COPCHost* m_pHost;
	COPCServer* m_pServer;
};
