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
		result = CoInitialize(nullptr);
	}
	if (mode == MULTITHREADED)
	{
		result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	}

	if (FAILED(result))
	{
		throw OPCException("CoInitialize failed", result);
	}

	CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

	return true;
}

bool OpcClientSDKImp::Uninitialize()
{
	CoUninitialize();
	return true;
}

bool OpcClientSDKImp::GetCLSIDList(const char* svrAddr, std::vector<std::string>& list)
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
		e.reasonString();
		bResult = false;
	}

	if (pHost)
	{
		delete pHost;
		pHost = NULL;
	}
	return bResult;
}

bool OpcClientSDKImp::ConnectServer(const char* svrAddr, const char* progid)
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
	}
	return false;
}

bool OpcClientSDKImp::DisConnectServer(const char* svrAddr, const char* progid)
{
	try
	{
		if (m_pHost)
		{
			delete m_pHost;
			m_pHost = NULL;
		}
	}
	catch (OPCException& e)
	{
		
	}
	return false;
}

bool OpcClientSDKImp::GetServerStatus(ServerStatus& status)
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
	}
	return false;
}

bool OpcClientSDKImp::GetItemsList(std::vector<std::string>& lstItems)
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
	}
	return false;
}

bool OpcClientSDKImp::AddGroup(const char* groupName, unsigned long& refreshRate)
{
	try
	{
		if (m_pServer)
		{
			COPCGroup *group = m_pServer->makeGroup(groupName, true, 1000, refreshRate, 0.0);
			m_pServer->AddGroupToMap(group);
			MyDataCallBack* pCallBack = new MyDataCallBack(m_ValueReport, m_pUser);
			group->enableAsynch(*pCallBack);
			return true;
		}
	}
	catch (OPCException& e)
	{
	}
	return false;
}

bool OpcClientSDKImp::RemoveGroup(const char* groupName)
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
	}
	return false;
}

bool OpcClientSDKImp::AddItems(const char* groupName, std::vector<std::string> lstAdded)
{
	try
	{
		if (m_pServer)
		{
			return m_pServer->AddItems(groupName, lstAdded);
		}
	}
	catch (OPCException& e)
	{
	}
	return false;
}

bool OpcClientSDKImp::RemoveItems(const char* groupName, std::vector<std::string> lstDel)
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
	}
	return false;
}

void OpcClientSDKImp::SetValueReport(OPCValueEventCallBack pFun,void* pUser)
{
	m_ValueReport = pFun;
	m_pUser = pUser;
}

void OpcClientSDKImp::GetLastOPCError(std::string& szText, long& code)
{
	szText = g_ErrorText;
	code = g_ErrorCode;
}

bool OpcClientSDKImp::WriteOPCValue(const char* groupName, const char* itemName, VARIANT& var)
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
