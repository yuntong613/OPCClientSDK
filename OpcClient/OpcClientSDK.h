#pragma once

#include "opcstructs.h"
#include <string>
class COPCHost;

class OpcClientSDK
{
public:
	OpcClientSDK();
	~OpcClientSDK();

	bool InitOPCSdk(OPCOLEInitMode mode = APARTMENTTHREADED);

	bool DestroyOPCSdk();

	COPCHost * makeHost(const std::string &hostName);
protected:
	COPCHost* m_pHost;
};
