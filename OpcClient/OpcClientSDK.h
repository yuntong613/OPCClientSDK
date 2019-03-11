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
	//��ʼ��COM����
	bool InitOPCSdk(OPCOLEInitMode mode = APARTMENTTHREADED);
	//ж��COM����
	bool DestroyOPCSdk();
	//��ȡ������OPC2.0�����б�
	void GetCLSIDList(const char* svrAddr, std::vector<std::string>& list);
	//����OPC����
	bool ConnectServer(const char* svrAddr, const char* progid);
	//�Ͽ�OPC����
	bool DisConnectServer(const char* svrAddr, const char* progid);
	//��ȡ����״̬
	bool GetServerStatus(ServerStatus& status);
	//��ȡ���
	bool GetItemsList(std::vector<std::string>& lstItems);
	//�����
	bool AddGroup(const char* name, unsigned long& refreshRate);
	//ɾ����
	bool RemoveGroup(const char* name);
	//��Ӷ�ȡ��
	bool AddItems(const char* group, std::vector<std::string> lstAdded);
	//ɾ����ȡ��
	bool RemoveItems(const char* group, std::vector<std::string> lstDel);

protected:
	COPCHost * makeHost(const std::string &hostName);
protected:
	COPCHost* m_pHost;
	COPCServer* m_pServer;
};
