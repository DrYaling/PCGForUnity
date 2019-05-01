#include "MovementSystem.h"

namespace ecs
{
	MovementSystem::MovementSystem()
	{
	}

	MovementSystem::~MovementSystem()
	{
		LogFormat("MovementSystem::~MovementSystem()");
	}

	SystemCatalog MovementSystem::GetCatalog()
	{
		return SystemCatalog::MOVEMENT;
	}

	void MovementSystem::OnUpdate(int32_t time_diff)
	{
		LogFormat("MovementSystem::OnUpdate(int32_t %d),pririoty %d", time_diff, GetPriority());
		for (auto itr = m_aData.begin(); itr != m_aData.end(); itr++)
		{
			//LogFormat("addr %d,first element %d",itr,&itr->GetID());
			if (itr->IsDirty())
			{
				//do
				LogFormat("MovementSystem com %d is dirty", itr->GetID());
				FlushComponent(itr);
			}
		}
	}
}