#include "stdafx.h"
#include "MyDataCallBack.h"
#include "OPCGroup.h"
#include "OPCItem.h"

MyDataCallBack::MyDataCallBack(OPCValueEventCallBack pCallBack, void* pUser)
{
	m_pFunc = pCallBack;
	m_pUser = pUser;
}

void MyDataCallBack::OnDataChange(COPCGroup & group, CAtlMap<COPCItem *, OPCItemData *> & changes)
{
	if (m_pFunc)
	{
		POSITION nPos = changes.GetStartPosition();
		while (nPos)
		{
			CAtlMap<COPCItem*,OPCItemData*>::CPair* p = changes.GetNext(nPos);
			if (p)
			{
				COPCItem* pItem = p->m_key;
				OPCItemData* pData = p->m_value;
				if (pItem && pData)
				{
					m_pFunc((char*)group.getName().c_str(), (char*)pItem->getName().c_str(), pData->vDataValue, pData->wQuality, m_pUser);
				}
			}
		}
	}
}
