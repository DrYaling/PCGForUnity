#include "SystemContainer.h"
#include "Logger/Logger.h"
#include "MovementSystem.h"
#include "StatusSystem.h"
SystemContainer::SystemContainer()
{
}

SystemContainer::~SystemContainer()
{
	ISystemInterface* system = nullptr;
	m_lSystem.clear(true);
	m_lSystem.ReadReset();
	while (ISystemInterface*system = m_lSystem.ReadNext())
	{
		delete system;
	}
}

void SystemContainer::Initilize()
{
}

bool SystemContainer::AddSystem(SystemCatalog catalog, int priority)
{
	m_lSystem.ReadReset();
	ISystemInterface* system;
	while ( system = m_lSystem.ReadNext())
	{
		if (system->GetCatalog() == catalog)
		{
			m_lSystem.ReadReset();
			return false;
		}
	}
	m_lSystem.ReadReset();
	switch (catalog)
	{
	case  SystemCatalog::MOVEMENT:
	{
		system = new MovementSystem();
		system->m_nPriority = priority;
		system->SetEnabled(true);
		m_lSystem.add(dynamic_cast<ISystemInterface*>(system), priority);
		return true;
	}
	break;
	case  SystemCatalog::STATUS:
	{
		system = new StatusSystem();
		system->m_nPriority = priority;
		system->SetEnabled(true);
		m_lSystem.add(dynamic_cast<ISystemInterface*>(system), priority);
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
	m_lSystem.ReadReset();
	LogFormat("SystemContainer::OnUpdate %d ,system size %d", time_diff, m_lSystem.size());
	while (ISystemInterface* system = m_lSystem.ReadNext())
	{
		if (system->GetEnabled())
		{
			system->OnUpdate(time_diff);
		}
	}
	m_lSystem.ReadReset();
}

void SystemContainer::UnRegisterComponent(int componentId, SystemCatalog catalog)
{
}

void SystemContainer::SetPriority(SystemCatalog catalog, int32_t priority)
{
	m_lSystem.ReadReset();
	while (ISystemInterface* system = m_lSystem.ReadNext())
	{
		if (system->GetCatalog() == catalog)
		{
			m_lSystem.ReadReset();
			if (m_lSystem.ResetPriority(system, priority))
				system->m_nPriority = priority;
			return;
		}
	}
	m_lSystem.ReadReset();
}
