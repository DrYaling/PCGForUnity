#ifndef I_System_h
#define I_System_h
#include "define.h"
#include "IComponent.h"
#include "G3D/Array.h"
#include "Logger/Logger.h"
#include <functional>
#include <map>

namespace ecs
{
	//using namespace G3D;
	/*
	enum class SystemCatalog :uint16_t
	{
		MOVEMENT = (uint16_t)ComponentCatalog::MOVEMENT,
		STATUS = (uint16_t)ComponentCatalog::STATUS,
	};*/
	typedef ComponentCatalog SystemCatalog;
	class SystemContainer;
	typedef std::function<void(IComponent*, uint32_t, ComponentCatalog)> ComponentChangeEvent;
	/*
	非线程安全ECS系统接口
	*/
	class ISystem {
		friend SystemContainer;
	public:
		virtual void OnUpdate(int32_t time_diff) = 0;
		ISystem() {}
		virtual ~ISystem()
		{
			LogFormat("ISystem::~ISystem()");
		}
		virtual SystemCatalog GetCatalog() = 0;
		int32_t GetPriority() { return m_nPriority; }
		int32_t GetPriority() const { return m_nPriority; }
		void SetEnabled(bool yes) { m_bEnable = yes; }
		bool GetEnabled() { return m_bEnable; }
	protected:
		void FlushComponent(IComponent* com)
		{
			if (com)
			{
				com->SetDirty(false);
			}
		}
	protected:
		int32_t m_nPriority;
		bool m_bEnable;
	};
	/*
	非线程安全ECS系统模板
	*/
	template<class T>
	class System
	{
		friend SystemContainer;
		typedef std::map<uint32_t, ComponentChangeEvent> ComponentEventMap;
		typedef const ComponentEventMap::iterator ConstComponentEventMapItr;
	public:
		System() { m_mEventMap.clear(); }
		virtual ~System()
		{
			LogFormat("System::~System()");
		}
		void RegisterComponentChangeEvent(ComponentChangeEvent event_call, uint32_t componentId)
		{
			auto itr = m_mEventMap.find(componentId);
			ConstComponentEventMapItr end = m_mEventMap.end();
			if (itr != end)
			{
				LogWarningFormat("component %d already registered event,replace old one", componentId);
				m_mEventMap[componentId] = event_call;
			}
			else
			{
				m_mEventMap.insert(std::make_pair(componentId, event_call));
			}
		}
		void UnRegisterComponentChangeEvent(uint32_t componentId)
		{
			auto itr = m_mEventMap.find(componentId);
			ConstComponentEventMapItr end = m_mEventMap.end();
			if (itr != end)
			{
				m_mEventMap.erase(itr);
			}
		}
		void RegisterComponent(const T& com)
		{
			m_aData.append(com);
			ConstComponentEventMapItr end = m_mEventMap.end();
			for (auto com = m_aData.begin(); com != m_aData.end(); com++)
			{
				ConstComponentEventMapItr event_call_itr = m_mEventMap.find(com->GetID());
				if (event_call_itr != end)
				{
					event_call_itr->second(com, com->GetID(), com->GetCatalog());
				}
			}
		}
		void UnRegisterComponent(int componentId)
		{
			for (auto itr = m_aData.begin(); itr != m_aData.end(); itr++)
			{
				if (itr->GetID() == componentId)
				{
					ConstComponentEventMapItr end = m_mEventMap.end();
					ConstComponentEventMapItr removed = m_mEventMap.find(componentId);
					if (removed != end)
					{
						removed->second(nullptr, componentId, itr->GetCatalog());
					}
					m_aData.remove(itr);
					for (auto com = m_aData.begin(); com != m_aData.end(); com++)
					{
						ConstComponentEventMapItr event_call_itr = m_mEventMap.find(com->GetID());
						if (event_call_itr != end)
						{
							event_call_itr->second(com, com->GetID(), com->GetCatalog());
						}
					}
					return;
				}
			}
		}
		bool operator == (const System& right) const
		{
			return this == &right;
		}
	protected:
		G3D::Array<T> m_aData;
	private:
		ComponentEventMap m_mEventMap;
	};
	class ComponentIDGenerator
	{
	public:
		ComponentIDGenerator() = delete;
		static uint32_t GetComponentID();
	};
}
#endif
