#pragma once

#include <string>
enum OPCOLEInitMode { APARTMENTTHREADED, MULTITHREADED };

#ifndef OPCCLIENT_EXPORTS
typedef
enum tagOPCSERVERSTATE
{
	OPC_STATUS_RUNNING = 1,
	OPC_STATUS_FAILED = (OPC_STATUS_RUNNING + 1),
	OPC_STATUS_NOCONFIG = (OPC_STATUS_FAILED + 1),
	OPC_STATUS_SUSPENDED = (OPC_STATUS_NOCONFIG + 1),
	OPC_STATUS_TEST = (OPC_STATUS_SUSPENDED + 1),
	OPC_STATUS_COMM_FAULT = (OPC_STATUS_TEST + 1)
} 	OPCSERVERSTATE;
#endif

/**
* Holds status information about the server
*/
typedef struct {
	FILETIME ftStartTime;
	FILETIME ftCurrentTime;
	FILETIME ftLastUpdateTime;
	OPCSERVERSTATE dwServerState;
	DWORD dwGroupCount;
	DWORD dwBandWidth;
	WORD wMajorVersion;
	WORD wMinorVersion;
	WORD wBuildNumber;
	std::string vendorInfo;
} ServerStatus;

//值更新回调声明
typedef void(*OPCValueEventCallBack)(char* strNodeName, VARIANT& vValue, int nQuailty, void* pUser);