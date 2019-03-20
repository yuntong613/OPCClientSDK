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
	virtual bool GetCLSIDList(const char* svrAddr, std::vector<std::string>& list);
	//����OPC����
	virtual bool ConnectServer(const char* svrAddr, const char* progid);
	//�Ͽ�OPC����
	virtual bool DisConnectServer(const char* svrAddr, const char* progid);
	//��ȡ����״̬
	virtual bool GetServerStatus(ServerStatus& status);
	//��ȡ���
	virtual bool GetItemsList(std::vector<std::string>& lstItems);
	//�����
	virtual bool AddGroup(const char* groupName, unsigned long& refreshRate);
	//ɾ����
	virtual bool RemoveGroup(const char* groupName);
	//��Ӷ�ȡ��
	virtual bool AddItems(const char* groupName, std::vector<std::string> lstAdded);
	//ɾ����ȡ��
	virtual bool RemoveItems(const char* groupName, std::vector<std::string> lstDel);

	virtual bool WriteOPCValue(const char* groupName, const char* itemName, VARIANT& var);

	virtual void SetValueReport(OPCValueEventCallBack pFun, void* pUser);

	virtual void GetLastOPCError(std::string& szText, long& code);

protected:
	COPCHost * makeHost(const std::string &hostName);
protected:
	COPCHost* m_pHost;
	COPCServer* m_pServer;
	OPCValueEventCallBack m_ValueReport;
	void* m_pUser;

	MyDataCallBack* m_pCallBack;
};
