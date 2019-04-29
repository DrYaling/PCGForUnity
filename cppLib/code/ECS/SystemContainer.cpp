#include "SystemContainer.h"
#include "Logger/Logger.h"
#include "MovementSystem.h"
SystemContainer::SystemContainer()
{
}

SystemContainer::~SystemContainer()
{
	int32_t index = 0;
	ISystem* system = nullptr;
	while (system = m_lSystem.GetAtIndex(index++))
	{
		delete system;
	}
}

void SystemContainer::Initilize()
{
}

bool SystemContainer::AddSystem(SystemCatalog catalog, int priority)
{
	int32_t index = 0;
	ISystem* system = nullptr;
	while (system = m_lSystem.GetAtIndex(index++))
	{
		if (system->GetCatalog() == catalog)
		{
			return false;
		}
	}
	switch (catalog)
	{
	case  SystemCatalog::MOVEMENT:
	{
		system = new MovementSystem();
		system->m_nPriority = priority;
		system->SetEnabled(true);
		m_lSystem.add(dynamic_cast<ISystem*>(system), priority);
		return true;
	}
	break;
	default:
		break;
	}
	return false;
}

void SystemContainer::OnUpdate(int32_t time_diff)
{
	int32_t index = 0;
	LogFormat("SystemContainer::OnUpdate %d ,system size %d", time_diff, m_lSystem.size());
	while (ISystem* system = m_lSystem.GetAtIndex(index++))
	{
		if (system->GetEnabled())
		{
			system->OnUpdate(time_diff);
		}
	}
}

void SystemContainer::UnRegisterComponent(int componentId, SystemCatalog catalog)
{
}

void SystemContainer::SetPriority(SystemCatalog catalog, int32_t priority)
{
	int32_t index = 0;
	while (ISystem* system = m_lSystem.GetAtIndex(index++))
	{
		if (system->GetCatalog() == catalog)
		{
			if (m_lSystem.ResetPriority(system, priority))
				system->m_nPriority = priority;
			return;
		}
	}
}
