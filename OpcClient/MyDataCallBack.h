#pragma once

#include "opcstructs.h"
#include "IAsynchDataCallback.h"

class MyDataCallBack : public IAsynchDataCallback
{
public:
	MyDataCallBack(OPCValueEventCallBack pCallBack, void* pUser);

	virtual void OnDataChange(COPCGroup & group, CAtlMap<COPCItem *, OPCItemData *> & changes) override;
protected:
	OPCValueEventCallBack m_pFunc;
	void* m_pUser;
};

