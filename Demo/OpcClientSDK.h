#pragma once

#include "opcstructs.h"
#include <vector>

class OPCSDKEXPORT OpcClientSDK
{
public:
	//��ʼ������
	virtual bool Initialize(OPCOLEInitMode mode = APARTMENTTHREADED) = 0;
	//ж�ػ���
	virtual bool Uninitialize() = 0;	
	//��ȡ������OPC2.0�����б�
	virtual bool GetCLSIDList(const char* svrAddr, std::vector<std::string>& list) = 0;
	//����OPC����
	virtual bool ConnectServer(const char* svrAddr, const char* progid) = 0;
	//�Ͽ�OPC����
	virtual bool DisConnectServer(const char* svrAddr, const char* progid) = 0;
	//��ȡ����״̬
	virtual bool GetServerStatus(ServerStatus& status) = 0;
	//��ȡ���
	virtual bool GetItemsList(std::vector<std::string>& lstItems) = 0;
	//�����
	virtual bool AddGroup(const char* groupName, unsigned long& refreshRate) = 0;
	//ɾ����
	virtual bool RemoveGroup(const char* groupName) = 0;
	//��Ӷ�ȡ��
	virtual bool AddItems(const char* groupName, std::vector<std::string> lstAdded) = 0;
	//ɾ����ȡ��
	virtual bool RemoveItems(const char* groupName, std::vector<std::string> lstDel) = 0;
	//дOPCֵ�첽
	virtual bool WriteOPCValue(const char* groupName, const char* itemName, VARIANT& var) = 0;
	//ֵ�ص�
	virtual void SetValueReport(OPCValueEventCallBack pFun, void* pUser) = 0;
	//��ȡ������Ϣ
	virtual void GetLastOPCError(std::string& szText, long& code) = 0;
public:
	static OpcClientSDK* CreateOPCSDK();
	static void DestroyOPCSDK(OpcClientSDK* pSdk);
};

