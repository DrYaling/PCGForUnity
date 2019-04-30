#include "StatusSystem.h"

StatusSystem::StatusSystem()
{
}

StatusSystem::~StatusSystem()
{
	LogFormat("StatusSystem::~StatusSystem()");
}

void StatusSystem::OnUpdate(int32_t time_diff)
{
	LogFormat("StatusSystem::OnUpdate(int32_t %d),pririoty %d", time_diff, GetPriority());
	for (auto itr = m_aData.begin(); itr != m_aData.end(); itr++)
	{
		//LogFormat("addr %d,id %d,status %d", itr, itr->GetID(),&itr->status);
		if (itr->IsDirty())
		{
			//do
			LogFormat("StatusSystem com %d is dirty",itr->GetID());
			FlushComponent(itr);
		}
	}
}
SystemCatalog StatusSystem::GetCatalog()
{
	return SystemCatalog::STATUS;
}
