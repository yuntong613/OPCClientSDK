#include "stdafx.h"
#include <utility>
#include "OpcClientSDK.h"
#include "opcda.h"
#include "OPCException.h"
#include "OPCHost.h"
#include "OPCServer.h"
#include "OPCGroup.h"


OpcClientSDK::OpcClientSDK()
{
	m_pHost = NULL;
	m_pServer = NULL;
}

OpcClientSDK::~OpcClientSDK()
{
	if (m_pHost)
	{
		delete m_pHost;
		m_pHost = NULL;
	}
}

bool OpcClientSDK::InitOPCSdk(OPCOLEInitMode mode /*= APARTMENTTHREADED*/)
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

bool OpcClientSDK::DestroyOPCSdk()
{
	CoUninitialize();
	return true;
}

void OpcClientSDK::GetCLSIDList(const char* svrAddr, std::vector<std::string>& list)
{
	COPCHost* pHost = NULL;
	try
	{
		pHost = makeHost(svrAddr);
		std::auto_ptr<COPCHost> p(pHost);
		p->getListOfDAServers(IID_CATID_OPCDAServer20, list);
	}
	catch (OPCException& e)
	{
		e.reasonString();
	}
}

bool OpcClientSDK::ConnectServer(const char* svrAddr, const char* progid)
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

bool OpcClientSDK::DisConnectServer(const char* svrAddr, const char* progid)
{
	try
	{
		if (m_pServer)
		{
			delete m_pServer;
			m_pServer = NULL;
		}
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

bool OpcClientSDK::GetServerStatus(ServerStatus& status)
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

bool OpcClientSDK::GetItemsList(std::vector<std::string>& lstItems)
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

bool OpcClientSDK::AddGroup(const char* name, unsigned long& refreshRate)
{
	try
	{
		if (m_pServer)
		{
			COPCGroup *group = m_pServer->makeGroup(name, true, 1000, refreshRate, 0.0);
			m_pServer->AddGroupToMap(group);
			return true;
		}
	}
	catch (OPCException& e)
	{
	}
	return false;
}

bool OpcClientSDK::RemoveGroup(const char* name)
{
	try
	{
		if (m_pServer)
		{
			m_pServer->RemoveGroupFromMap(name);
			return true;
		}
	}
	catch (OPCException& e)
	{
	}
	return false;
}

bool OpcClientSDK::AddItems(const char* group, std::vector<std::string> lstAdded)
{
	try
	{
		if (m_pServer)
		{
			return m_pServer->AddItems(group, lstAdded);
		}
	}
	catch (OPCException& e)
	{
	}
	return false;
}

bool OpcClientSDK::RemoveItems(const char* group, std::vector<std::string> lstDel)
{
	try
	{
		if (m_pServer)
		{
			return m_pServer->RemoveItems(group, lstDel);
		}
	}
	catch (OPCException& e)
	{
	}
	return false;
}

COPCHost * OpcClientSDK::makeHost(const std::string &hostName)
{
	COPCHost* pHost = NULL;
	if (hostName.size() == 0) {
		pHost = new CLocalHost;
	}
	else {
		pHost = new CRemoteHost(hostName);
	}
	return pHost;
}
