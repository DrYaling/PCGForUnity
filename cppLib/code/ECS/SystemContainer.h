#ifndef System_container_h
#define System_container_h
#include "define.h"
#include "ISystem.h"
#include "Utinities/PriorityList.h"
#include <mutex>
namespace ecs
{
	enum class SystemGroup
	{
		NONE = 0,
		SERVER_WORLD,
		CLIENT_WORLD,
		SERVER_AI,
		CLIENT_AI,
		SERVER_SKILL_SYSTEM,
	};
	class MovementSystem;
	class SystemContainer
	{
	public:
		SystemContainer();
		~SystemContainer();
		void SetGroup(SystemGroup group) { m_eGroup = group; }
		SystemGroup GetGroup() { return m_eGroup; }
		void Initilize(SystemGroup group);
		bool AddSystem(SystemCatalog catalog, int priority = 0);
		void OnUpdate(int32_t time_diff);
		template<class T>
		void RegisterComponent(const T& com, SystemCatalog catalog);
		void UnRegisterComponent(int componentId, SystemCatalog catalog);
		void SetPriority(SystemCatalog catalog, int32_t priority);
		template<class T>
		void RegisterComponentChangeEvent(ComponentChangeEvent event_call, uint32_t componentId, SystemCatalog catalog)
		{
			auto pSys = GetSystem<T>(catalog);
			if (pSys)
			{
				std::lock_guard<std::mutex> lock(m_systemMtx);
				pSys->RegisterComponentChangeEvent(event_call, componentId);
			}
		}
		template<class T>
		void UnRegisterComponentChangeEvent(uint32_t componentId, SystemCatalog catalog)
		{
			auto pSys = GetSystem<T>(catalog);
			if (pSys)
			{
				pSys->UnRegisterComponentChangeEvent(componentId);
			}
		}
	private:
		const ISystem * GetSystemInterface(SystemCatalog catalog)
		{
			for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
			{
				if (itr->GetCatalog() == catalog)
				{
					return &*itr;
				}
			}
			return nullptr;
		}
		template<class T>
		System<T> * GetSystem(SystemCatalog catalog)
		{
			for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
			{
				if (itr->GetCatalog() == catalog)
				{
					return dynamic_cast<System<T>*>(itr.ptr());;
				}
			}
			return nullptr;
		}
	private:
		priorityList<ISystem> m_lSystem;
		SystemGroup m_eGroup;
		std::mutex m_systemMtx;
	};

	template<class T>
	void SystemContainer::RegisterComponent(const T & com, SystemCatalog catalog)
	{
		for (auto itr = m_lSystem.begin(); itr != m_lSystem.end(); itr++)
		{
			if (itr->GetCatalog() == catalog)
			{
				System<T>* sys = dynamic_cast<System<T>*>(itr.ptr());
				if (sys)
				{
					std::lock_guard<std::mutex> lock(m_systemMtx);
					return sys->RegisterComponent(com);
				}
			}
		}
	}
}
#endif