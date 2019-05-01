#include "SystemContainer.h"
#include "Logger/Logger.h"
#include "MovementSystem.h"
#include "StatusSystem.h"
namespace ecs
{
	SystemContainer::SystemContainer() :
		m_eGroup(SystemGroup::NONE)
	{
	}

	SystemContainer::~SystemContainer()
	{
		ISystem* system = nullptr;
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end();)
		{
			delete itr.ptr();
			itr = m_lSystem.erase(itr);
		}
	}

	void SystemContainer::Initilize()
	{
	}

	bool SystemContainer::AddSystem(SystemCatalog catalog, int priority)
	{
		ISystem* system = nullptr;
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
		{
			if (itr->GetCatalog() == catalog)
			{
				return false;
			}
		}
		bool ret = false;
		switch (catalog)
		{
			case  SystemCatalog::MOVEMENT:
				{
					system = new MovementSystem();
					system->m_nPriority = priority;
					system->SetEnabled(true);
					m_lSystem.add(dynamic_cast<ISystem*>(system), priority);
					ret = true;
				}
				break;
			case  SystemCatalog::STATUS:
				{
					system = new StatusSystem();
					system->m_nPriority = priority;
					system->SetEnabled(true);
					m_lSystem.add(dynamic_cast<ISystem*>(system), priority);
					ret = true;
				}
				break;
			default:
				break;
		}
		return ret;
	}

	void SystemContainer::OnUpdate(int32_t time_diff)
	{
		LogFormat("SystemContainer::OnUpdate %d ,system size %d", time_diff, m_lSystem.size());
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
		{
			if (itr->GetEnabled())
			{
				itr->OnUpdate(time_diff);
			}
		}
	}

	void SystemContainer::UnRegisterComponent(int componentId, SystemCatalog catalog)
	{
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
		{
			if (itr->GetCatalog() == catalog)
			{
				switch (catalog)
				{
					case SystemCatalog::MOVEMENT:
						{
							System<MovementComponent>* sys = dynamic_cast<System<MovementComponent>*>(itr.ptr());
							if (sys)
							{
								return sys->UnRegisterComponent(componentId);
							}
						}
						break;
					case SystemCatalog::STATUS:
						{
							System<StatusComponent>* sys = dynamic_cast<System<StatusComponent>*>(itr.ptr());
							if (sys)
							{
								return sys->UnRegisterComponent(componentId);
							}
						}
						break;
					default:
						break;
				}
			}

		}
	}
	void SystemContainer::SetPriority(SystemCatalog catalog, int32_t priority)
	{
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
		{
			if (itr->GetCatalog() == catalog)
			{
				if (m_lSystem.ResetPriority(itr, priority))
					itr->m_nPriority = priority;
				return;
			}
		}
	}
}