#pragma once

class COPCGroup;
class COPCItem;

#include <atlcoll.h>
#include "OPCItemData.h"

class IAsynchDataCallback
{
public:
	virtual void OnDataChange(COPCGroup & group, CAtlMap<COPCItem *, OPCItemData *> & changes) = 0;
};

