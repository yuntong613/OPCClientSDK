#include "stdafx.h"
#include "OpcClientSDK.h"
#include "OpcClientSDKImp.h"


OpcClientSDK* OpcClientSDK::CreateOPCSDK()
{
	return new OpcClientSDKImp;
}

void OpcClientSDK::DestroyOPCSDK(OpcClientSDK* pSdk)
{
	OpcClientSDKImp* p = (OpcClientSDKImp*)pSdk;
	if (p)
	{
		delete p;
		p = NULL;
	}
}
