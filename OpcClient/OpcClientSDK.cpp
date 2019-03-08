#include "stdafx.h"
#include "OpcClientSDK.h"
#include "opcda.h"
#include "OPCException.h"
#include "OPCHost.h"

OpcClientSDK::OpcClientSDK()
{
	m_pHost = NULL;
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

COPCHost * OpcClientSDK::makeHost(const std::string &hostName)
{
	if (hostName.size() == 0) {
		m_pHost = new CLocalHost;
	}
	else {
		m_pHost = new CRemoteHost(hostName);
	}
	return m_pHost;
}
