#include "MovementSystem.h"

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
	LogFormat("MovementSystem::OnUpdate(int32_t %d)",time_diff);
	for (auto itr = m_aData.begin();itr!= m_aData.end();itr++)
	{
		if (itr->IsDirty())
		{
			//do
		}
	}
}
