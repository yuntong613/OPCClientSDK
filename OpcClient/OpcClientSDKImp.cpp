#include "stdafx.h"
#include <utility>
#include "OpcClientSDKImp.h"
#include "opcda.h"
#include "OPCException.h"
#include "OPCHost.h"
#include "OPCServer.h"
#include "OPCGroup.h"

std::string g_ErrorText;
HRESULT g_ErrorCode;

OpcClientSDKImp::OpcClientSDKImp()
{
	m_pHost = NULL;
	m_pServer = NULL;
	m_pUser = NULL;
	m_ValueReport = NULL;
	m_pCallBack = NULL;
}

OpcClientSDKImp::~OpcClientSDKImp()
{
	if (m_pHost)
	{
		delete m_pHost;
		m_pHost = NULL;
	}
}

bool OpcClientSDKImp::Initialize(OPCOLEInitMode mode /*= APARTMENTTHREADED*/)
{
	HRESULT result;
	if (mode == APARTMENTTHREADED)
	{
		result = CoInitialize(NULL);
	}
	if (mode == MULTITHREADED)
	{
		result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	}

// 	if (result != S_OK)
// 	{
// 		throw OPCException("CoInitialize failed", result);
// 	}

	CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

	return true;
}

bool OpcClientSDKImp::Uninitialize()
{
	CoUninitialize();
	return true;
}

bool OpcClientSDKImp::GetCLSIDList(const char* svrAddr, std::vector<std::string>& list, OPCException* ex /*= NULL*/)
{
	bool bResult = false;
	COPCHost* pHost = NULL;
	try
	{
		pHost = makeHost(svrAddr);	
		pHost->getListOfDAServers(IID_CATID_OPCDAServer20, list);
		bResult = true;
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
		bResult = false;
	}

	if (pHost)
	{
		delete pHost;
		pHost = NULL;
	}
	return bResult;
}

bool OpcClientSDKImp::ConnectServer(const char* svrAddr, const char* progid, OPCException* ex/*=NULL*/)
{
	try
	{
		m_pHost = makeHost(svrAddr);
		if (m_pHost)
		{
			m_pServer = m_pHost->connectDAServer(progid);
			return true;
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return false;
}

bool OpcClientSDKImp::DisConnectServer(const char* svrAddr, const char* progid, OPCException* ex/*=NULL*/)
{
	try
	{
		if (m_pHost)
		{
			delete m_pHost;
			m_pHost = NULL;
		}
		m_pServer = NULL;//COPCHost析构函数中执行delete，此处需置NULL added by fangqing

	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return true;
}

bool OpcClientSDKImp::GetServerStatus(ServerStatus& status, OPCException* ex /*= NULL*/)
{
	try
	{
		if (m_pServer)
		{
			m_pServer->getStatus(status);
			return true;
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return false;
}

bool OpcClientSDKImp::GetItemsList(std::vector<std::string>& lstItems, OPCException* ex/*= NULL*/)
{
	try
	{
		if (m_pServer)
		{
			m_pServer->getItemNames(lstItems);
			return true;
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return false;
}

bool OpcClientSDKImp::AddGroup(const char* groupName, unsigned long& refreshRate, OPCException* ex/*= NULL*/)
{
	COPCGroup *group = NULL;
	try
	{
		if (m_pServer)
		{
			//判断组名是否存在
			if (m_pServer->exist_group(groupName)|| strlen(groupName)<=0)
			{
				return false;
			}
			group = m_pServer->makeGroup(groupName, true, 1000, refreshRate, 0.0);
			
			MyDataCallBack* pCallBack = new MyDataCallBack(m_ValueReport, m_pUser);
			group->enableAsynch(*pCallBack);

			m_pServer->AddGroupToMap(group);
			return true;
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
		if (group)
		{
			delete group;
			group = NULL;
		}
	}

	return false;
}

bool OpcClientSDKImp::RemoveGroup(const char* groupName, OPCException* ex/*= NULL*/)
{
	try
	{
		if (m_pServer)
		{
			m_pServer->RemoveGroupFromMap(groupName);
			return true;
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return false;
}

bool OpcClientSDKImp::AddItems(const char* groupName, std::vector<std::string> lstAdded, std::vector<long>& errors, std::vector<VARTYPE>& dataTypes, OPCException* ex/*= NULL*/)
{
	try
	{
		if (m_pServer)
		{
			return m_pServer->AddItems(groupName, lstAdded, errors, dataTypes);
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return false;
}

bool OpcClientSDKImp::RemoveItems(const char* groupName, std::vector<std::string> lstDel, OPCException* ex/*= NULL*/)
{
	try
	{
		if (m_pServer)
		{
			return m_pServer->RemoveItems(groupName, lstDel);
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return false;
}

void OpcClientSDKImp::SetValueReport(OPCValueEventCallBack pFun,void* pUser)
{
	m_ValueReport = pFun;
	m_pUser = pUser;
}

bool OpcClientSDKImp::WriteOPCValue(const char* groupName, const char* itemName, VARIANT& var, OPCException* ex/*= NULL*/)
{
	try
	{
		if (m_pServer)
		{
			return m_pServer->WriteOPCValue(groupName, itemName, var);
		}
	}
	catch (OPCException& e)
	{
		if (ex)
		{
			ex->reasonString(e.reasonString());
			ex->reasonCode(e.reasonCode());
		}
	}
	return false;
}

COPCHost * OpcClientSDKImp::makeHost(const std::string &hostName)
{
	COPCHost* pHost = NULL;
	if (hostName.size() == 0 || hostName.compare("127.0.0.1") == 0) {
		pHost = new CLocalHost;
	}
	else {
		pHost = new CRemoteHost(hostName);
	}
	return pHost;
}
