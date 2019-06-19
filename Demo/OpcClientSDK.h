#pragma once

#include "OPCException.h"
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
	virtual bool GetCLSIDList(const char* svrAddr, std::vector<std::string>& list, OPCException* ex = NULL) = 0;
	//����OPC����
	virtual bool ConnectServer(const char* svrAddr, const char* progid, OPCException* ex = NULL) = 0;
	//�Ͽ�OPC����
	virtual bool DisConnectServer(const char* svrAddr, const char* progid, OPCException* ex = NULL) = 0;
	//��ȡ����״̬
	virtual bool GetServerStatus(ServerStatus& status, OPCException* ex = NULL) = 0;
	//��ȡ���
	virtual bool GetItemsList(std::vector<std::string>& lstItems, OPCException* ex = NULL) = 0;
	//�����
	virtual bool AddGroup(const char* groupName, unsigned long& refreshRate, OPCException* ex = NULL) = 0;
	//ɾ����
	virtual bool RemoveGroup(const char* groupName, OPCException* ex = NULL) = 0;
	//��Ӷ�ȡ��
	virtual bool AddItems(const char* groupName, std::vector<std::string> lstAdded, std::vector<long>& errors, std::vector<VARTYPE>& dataTypes, OPCException* ex = NULL) = 0;
	//ɾ����ȡ��
	virtual bool RemoveItems(const char* groupName, std::vector<std::string> lstDel, OPCException* ex = NULL) = 0;
	//дOPCֵ�첽
	virtual bool WriteOPCValue(const char* groupName, const char* itemName, VARIANT& var, OPCException* ex = NULL) = 0;
	//ֵ�ص�
	virtual void SetValueReport(OPCValueEventCallBack pFun, void* pUser) = 0;
public:
	static OpcClientSDK* CreateOPCSDK();
	static void DestroyOPCSDK(OpcClientSDK* pSdk);
};

