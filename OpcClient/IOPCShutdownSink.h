#pragma once
#include "opccomn.h"

class COPCServer;

class IOPCShutdownSink :	public IOPCShutdown
{

public:
	IOPCShutdownSink(COPCServer* pServer);
	~IOPCShutdownSink();

	virtual HRESULT STDMETHODCALLTYPE ShutdownRequest(LPCWSTR szReason);

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef(void);

	virtual ULONG STDMETHODCALLTYPE Release(void);

protected:
	COPCServer* m_pServer;
	DWORD m_cnRef;
};

