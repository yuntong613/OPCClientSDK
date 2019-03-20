#include "stdafx.h"
#include "IOPCShutdownSink.h"
#include "OPCServer.h"

IOPCShutdownSink::IOPCShutdownSink(COPCServer* pServer)
{
	m_pServer = pServer;
}

IOPCShutdownSink::~IOPCShutdownSink()
{

}

HRESULT STDMETHODCALLTYPE IOPCShutdownSink::ShutdownRequest(LPCWSTR szReason)
{
	TCHAR sReason[1024] = { 0 };
	_wcstombsz(sReason, szReason, sizeof(sReason) / sizeof(TCHAR));

	m_pServer->ShutdownRequest(sReason);

	return (S_OK);
}

HRESULT STDMETHODCALLTYPE IOPCShutdownSink::QueryInterface(REFIID riid, void **ppvObject)
{
	// Validate ppInterface.  Return with "invalid argument" error code if invalid:
	if (ppvObject == NULL)
		return (E_INVALIDARG);

	*ppvObject = NULL;

	if (riid == IID_IUnknown)
		*ppvObject = (IUnknown *)this;
	else if (riid == IID_IOPCShutdown)
		*ppvObject = (IOPCShutdown *)this;
	else
	{

		return (E_NOINTERFACE);
	}

	AddRef();

	// Return with "success" code:
	return (S_OK);
}

ULONG STDMETHODCALLTYPE IOPCShutdownSink::AddRef(void)
{
	return (++m_cnRef);
}

ULONG STDMETHODCALLTYPE IOPCShutdownSink::Release(void)
{
	if (--m_cnRef != 0)
		return (m_cnRef);

	delete this;

	return (0);
}
