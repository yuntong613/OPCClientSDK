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
	//��ʼ��COM����
	virtual bool Initialize(OPCOLEInitMode mode = APARTMENTTHREADED);
	//ж��COM����
	virtual bool Uninitialize();
	//��ȡ������OPC2.0�����б�
	virtual bool GetCLSIDList(const char* svrAddr, std::vector<std::string>& list, OPCException* ex = NULL);
	//����OPC����
	virtual bool ConnectServer(const char* svrAddr, const char* progid, OPCException* ex=NULL);
	//�Ͽ�OPC����
	virtual bool DisConnectServer(const char* svrAddr, const char* progid, OPCException* ex=NULL);
	//��ȡ����״̬
	virtual bool GetServerStatus(ServerStatus& status, OPCException* ex = NULL);
	//��ȡ���
	virtual bool GetItemsList(std::vector<std::string>& lstItems, OPCException* ex= NULL);
	//�����
	virtual bool AddGroup(const char* groupName, unsigned long& refreshRate, OPCException* ex= NULL);
	//ɾ����
	virtual bool RemoveGroup(const char* groupName, OPCException* ex= NULL);
	//��Ӷ�ȡ��
	virtual bool AddItems(const char* groupName, std::vector<std::string> lstAdded, OPCException* ex= NULL);
	//ɾ����ȡ��
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
