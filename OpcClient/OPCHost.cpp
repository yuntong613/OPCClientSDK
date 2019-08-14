/*
OPCClientToolKit
Copyright (C) 2005 Mark C. Beharrell

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"
#include "OPCHost.h"
#include "OPCServer.h"
#include "opcda.h"
#include "OpcEnum.h"
#include <iostream>
#include "OPCException.h"
#include "IOPCShutdownSink.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COPCHost::COPCHost()
{
	m_pServer = NULL;
	m_pIShutdownSink = NULL;
	m_dwCookieShutdownSink = NULL;
}

COPCHost::~COPCHost()
{
	if (m_pServer)
	{
		delete m_pServer;
		m_pServer = NULL;
	}

	if (iConnPtContainer)
	{
		// Unadvise shutdown notifications:
		if (m_dwCookieShutdownSink != 0)
		{
			HRESULT hr = E_FAIL;
			IConnectionPoint *pCP = NULL;

			hr = getConnectionPointContainerInterface()->FindConnectionPoint(IID_IOPCShutdown, &pCP);

			if (SUCCEEDED(hr))
			{
				hr = pCP->Unadvise(m_dwCookieShutdownSink);
				pCP->Release();
			}

			if (FAILED(hr))
			{
				ATLTRACE(_T("OTC: CKServer::Disconnect () - failed to unadvise shutdown notifications\r\n"));
			}
			m_dwCookieShutdownSink = 0;
		}

		if (m_pIShutdownSink != NULL)
		{
			m_pIShutdownSink->Release();
			m_pIShutdownSink = NULL;
		}
	}
}


CRemoteHost::CRemoteHost(const std::string& hostName) : host(hostName)
{
}


void CRemoteHost::makeRemoteObject(const IID requestedClass, const IID requestedInterface, void** interfacePtr)
{
	COAUTHINFO athn;
	ZeroMemory(&athn, sizeof(COAUTHINFO));
	// Set up the NULL security information
	athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
	athn.dwAuthnSvc = RPC_C_AUTHN_WINNT;
	athn.dwAuthzSvc = RPC_C_AUTHZ_NONE;
	athn.dwCapabilities = EOAC_NONE;
	athn.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
	athn.pAuthIdentityData = NULL;
	athn.pwszServerPrincName = NULL;

	COSERVERINFO remoteServerInfo;
	ZeroMemory(&remoteServerInfo, sizeof(COSERVERINFO));
	remoteServerInfo.pAuthInfo = &athn;
//	remoteServerInfo.pAuthInfo = NULL;
	wstring hostName = CUtils::ANSIToUnicode(host);
	remoteServerInfo.pwszName = (wchar_t*)hostName.c_str();
	printf("%s\n", host.c_str());

	MULTI_QI reqInterface;
	reqInterface.pIID = &requestedInterface;
	reqInterface.pItf = NULL;
	reqInterface.hr = S_OK;

	HRESULT result = CoCreateInstanceEx(requestedClass, NULL, CLSCTX_SERVER, &remoteServerInfo, 1, &reqInterface);

	if (FAILED(result))
	{
		printf("Error %x\n", result);
		throw OPCException("CoCreateInstanceEx Failed to get remote interface", result);
	}

	*interfacePtr = reqInterface.pItf; // avoid ref counter getting incremented again
}


CLSID CRemoteHost::GetCLSIDFromRemoteRegistry(const std::string& hostName, const std::string& progID)
{
	ATL::CComPtr<IOPCServerList> iCatInfo;

	makeRemoteObject(CLSID_OpcServerList, IID_IOPCServerList, (void**)&iCatInfo);

	CATID Implist = IID_CATID_OPCDAServer20;

	ATL::CComPtr<IEnumCLSID> iEnum;
	HRESULT result = iCatInfo->EnumClassesOfCategories(1, &Implist, 0, NULL, &iEnum);
	if (FAILED(result))
	{
		throw OPCException("EnumClassesOfCategories Failed to get enum for categeories", result);
	}

	GUID glist = GUID_NULL;
	GUID classId = GUID_NULL;
	ULONG actual;
	while ((result = iEnum->Next(1, &glist, &actual)) == S_OK)
	{
		WCHAR* wprogID;
		WCHAR* userType;
		HRESULT res = iCatInfo->GetClassDetails(glist, &wprogID, &userType);/*ProgIDFromCLSID(glist, &progID)*/

		if (FAILED(res))
		{
			throw OPCException("Failed to get ProgId from ClassId", res);
		}
		else
		{
			std::string str = CUtils::UnicodeToANSI(wprogID);

			if (str.compare(progID) == 0)
			{
				classId = glist;
				CoTaskMemFree(wprogID);
				CoTaskMemFree(userType);
				break;
			}

			printf("ProgID: %s", str.c_str());

			printf("CLSID: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n",
				glist.Data1, glist.Data2, glist.Data3,
				glist.Data4[0], glist.Data4[1], glist.Data4[2], glist.Data4[3],
				glist.Data4[4], glist.Data4[5], glist.Data4[6], glist.Data4[7]);

			CoTaskMemFree(wprogID);
			CoTaskMemFree(userType);
		}
	}

	return classId;
}


COPCServer* CRemoteHost::connectDAServer(const std::string& serverProgIDOrClsID)
{
	const char* serverAppStr = serverProgIDOrClsID.c_str();

	CLSID clsid;

	if (serverAppStr[0] == '{') {
		std::wstring wStr(serverProgIDOrClsID.begin(), serverProgIDOrClsID.end());
		LPCOLESTR strClsId = wStr.c_str();

		HRESULT hr = CLSIDFromString(strClsId, &clsid);

		if (FAILED(hr))
			throw OPCException("Invalid CLSID string", hr);
	}
	else {
		clsid = GetCLSIDFromRemoteRegistry(host, serverProgIDOrClsID);
	}

	ATL::CComPtr<IUnknown> iUnknown;
	makeRemoteObject(clsid, IID_IUnknown, (void **)&iUnknown);

	ATL::CComPtr<IOPCServer> iOpcServer;
	HRESULT result = iUnknown->QueryInterface(IID_IOPCServer, (void**)&iOpcServer);
	if (FAILED(result))
	{
		throw OPCException("Failed obtain IID_IOPCServer interface from server", result);
	}

	if (m_pServer)
	{
		delete m_pServer;
		m_pServer = NULL;
	}
	m_pServer = new COPCServer(iOpcServer, serverProgIDOrClsID);

	makeRemoteObject(clsid, IID_IConnectionPointContainer, (void **)&iConnPtContainer);

	IConnectionPoint* cp = NULL;
	result = iConnPtContainer->FindConnectionPoint(IID_IOPCShutdown, &cp);
	if (SUCCEEDED(result))
	{
		try
		{
			// Instantiate the shutdown sink and add us to its reference count:
			m_pIShutdownSink = new IOPCShutdownSink(m_pServer);
			m_pIShutdownSink->AddRef();

			result = cp->Advise(m_pIShutdownSink, &m_dwCookieShutdownSink);

			// We are done with the connection point, so release our reference:
			cp->Release();
		}
		catch (...)
		{
			throw OPCException("Failed obtain IID_IOPCShutdown interface from server", result);
		}
	}
	return m_pServer;
}


void CRemoteHost::getListOfDAServers(CATID cid, std::vector<std::string>& listOfProgIDs)
{
	ATL::CComPtr<IOPCServerList> iCatInfo;
	
	makeRemoteObject(CLSID_OpcServerList, IID_IOPCServerList, (void**)(&iCatInfo));

	CATID Implist[1];
	Implist[0] = cid;

	ATL::CComPtr<IEnumCLSID> iEnum;
	HRESULT result = iCatInfo->EnumClassesOfCategories(1, Implist, 0, NULL, &iEnum);
	if (FAILED(result))
	{
		throw OPCException("Failed to get enum for categeories", result);
	}

	GUID glist;
	ULONG actual;
	while ((result = iEnum->Next(1, &glist, &actual)) == S_OK)
	{
		WCHAR* progID = NULL;
		WCHAR* userType = NULL;
		HRESULT res = iCatInfo->GetClassDetails(glist, &progID, &userType);/*ProgIDFromCLSID(glist, &progID)*/

		if (FAILED(res))
		{
			throw OPCException("Failed to get ProgId from ClassId", res);
		}
		else
		{
			std::string str = CUtils::UnicodeToANSI(progID);

			printf("ProgID: %s ", str.c_str());

			printf("CLSID: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n",
				glist.Data1, glist.Data2, glist.Data3,
				glist.Data4[0], glist.Data4[1], glist.Data4[2], glist.Data4[3],
				glist.Data4[4], glist.Data4[5], glist.Data4[6], glist.Data4[7]);

			listOfProgIDs.push_back(str);
			CoTaskMemFree(progID);
			CoTaskMemFree(userType);
		}
	}
}

CLSID CRemoteHost::getCLSID(const std::string& serverProgID)
{
	CLSID clsId;

	ATL::CComPtr<IOPCServerList> iCatInfo;

	makeRemoteObject(CLSID_OpcServerList, IID_IOPCServerList, (void**)&iCatInfo);

	std::wstring wStr(serverProgID.begin(), serverProgID.end());

	LPCOLESTR progId = wStr.c_str();

	HRESULT result = iCatInfo->CLSIDFromProgID(progId, &clsId);

	if (FAILED(result))
	{
		throw OPCException("Failed to get clsid", result);
	}

	return clsId;
}


CLocalHost::CLocalHost()
{
}


COPCServer* CLocalHost::connectDAServer(const std::string& serverProgID)
{

	wstring  wideName = CUtils::ANSIToUnicode(serverProgID);

	CLSID clsid;
	HRESULT result = CLSIDFromProgID((WCHAR*)wideName.c_str(), &clsid);
	if (FAILED(result))
	{
		throw OPCException("Failed to convert progID to class ID", result);
	}

	ATL::CComPtr<IClassFactory> iClassFactory;
	result = CoGetClassObject(clsid, CLSCTX_LOCAL_SERVER, NULL, IID_IClassFactory, (void**)&iClassFactory);
	if (FAILED(result))
	{
		throw OPCException("Failed get Class factory", result);
	}

	ATL::CComPtr<IUnknown> iUnknown;
	result = iClassFactory->CreateInstance(NULL, IID_IUnknown, (void**)&iUnknown);
	if (FAILED(result))
	{
		throw OPCException("Failed get create OPC server ref", result);
	}

	ATL::CComPtr<IOPCServer> iOpcServer;
	result = iUnknown->QueryInterface(IID_IOPCServer, (void**)&iOpcServer);
	if (FAILED(result))
	{
		throw OPCException("Failed obtain IID_IOPCServer interface from server", result);
	}
	if (m_pServer)
	{
		delete m_pServer;
		m_pServer = NULL;
	}
	m_pServer = new COPCServer(iOpcServer, serverProgID);

	result = iClassFactory->CreateInstance(NULL, IID_IConnectionPointContainer, (void**)&iConnPtContainer);

	IConnectionPoint* cp = NULL;
	result = iConnPtContainer->FindConnectionPoint(IID_IOPCShutdown, &cp);
	if (SUCCEEDED(result))
	{
		try
		{
			// Instantiate the shutdown sink and add us to its reference count:
			m_pIShutdownSink = new IOPCShutdownSink(m_pServer);
			m_pIShutdownSink->AddRef();

			result = cp->Advise(m_pIShutdownSink, &m_dwCookieShutdownSink);

			// We are done with the connection point, so release our reference:
			cp->Release();
		}
		catch (...)
		{
			throw OPCException("Failed obtain IID_IOPCShutdown interface from server", result);
		}
	}
	return m_pServer;
}


void CLocalHost::getListOfDAServers(CATID cid, std::vector<std::string>& listOfProgIDs)
{
	CATID Implist[1];
	Implist[0] = cid;
	ATL::CComPtr<ICatInformation> iCatInfo;

	HRESULT result = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatInformation, (void **)&iCatInfo);
	if (FAILED(result))
	{
		throw OPCException("Failed to get IID_ICatInformation", result);
	}

	ATL::CComPtr<IEnumCLSID> iEnum;
	result = iCatInfo->EnumClassesOfCategories(1, Implist, 0, NULL, &iEnum);
	if (FAILED(result))
	{
		throw OPCException("Failed to get enum for categeories", result);
	}

	GUID glist;
	ULONG actual;
	while ((result = iEnum->Next(1, &glist, &actual)) == S_OK)
	{
		WCHAR* progID;
		HRESULT res = ProgIDFromCLSID(glist, &progID);
		if (FAILED(res))
		{
			throw OPCException("Failed to get ProgId from ClassId", res);
		}
		else
		{
			std::string str = CUtils::UnicodeToANSI(progID);
			listOfProgIDs.push_back(str);
			CoTaskMemFree(progID);
		}
	}
}

CLSID CLocalHost::getCLSID(const std::string& serverProgID)
{
	CLSID clsId;

	std::wstring wStr(serverProgID.begin(), serverProgID.end());

	LPCOLESTR progId = wStr.c_str();

	HRESULT result =  CLSIDFromProgID(progId, &clsId);

	if (FAILED(result))
	{
		throw OPCException("Failed to get clsid", result);
	}

	return clsId;
}
