#ifndef I_System_h
#define I_System_h
#include "define.h"
#include "IComponent.h"
#include "StableArray.h"
#include "Logger/Logger.h"
#include <functional>
#include <map>
#define STABLE_TEST 1
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
		ISystem(): m_nPriority(0), m_bEnable(false)
		{
		}

		virtual ~ISystem()
		{
			LogFormat("ISystem::~ISystem()");
		}
		virtual SystemCatalog GetCatalog() = 0;
		int32_t GetPriority() { return m_nPriority; }
		int32_t GetPriority() const { return m_nPriority; }
		void SetEnabled(bool yes) { m_bEnable = yes; }
		bool GetEnabled() const { return m_bEnable; }
	protected:
		static void FlushComponent(IComponent* com)
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
		typedef ComponentEventMap::iterator ComponentEventMapItr;
	public:
		System() { m_mEventMap.clear(); }
		virtual ~System()
		{
			LogFormat("System::~System()");
		}
		void RegisterComponentChangeEvent(ComponentChangeEvent event_call, uint32_t componentId)
		{
			const auto itr = m_mEventMap.find(componentId);
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
			const auto itr = m_mEventMap.find(componentId);
			ConstComponentEventMapItr end = m_mEventMap.end();
			if (itr != end)
			{
				m_mEventMap.erase(itr);
			}
		}
		void RegisterComponent(const T& com)
		{
			T* ptr = m_aData.Add(com);
			ConstComponentEventMapItr end = m_mEventMap.end();
			if(m_aData.IsDirty())
			{
				for (auto itr = m_aData.begin(); itr != m_aData.end(); ++itr)
				{
					if(itr->GetInvalid())
						continue;
					ComponentEventMapItr event_call_itr = m_mEventMap.find(itr->GetID());
					if (event_call_itr != end)
					{
						event_call_itr->second(itr, itr->GetID(), itr->GetCatalog());
					}
				}
				m_aData.flush();
			}
			else
			{
				ComponentEventMapItr event_call_itr = m_mEventMap.find(com.GetID());
				if (event_call_itr != end)
				{
					event_call_itr->second(ptr, com.GetID(), com.GetCatalog());
				}
#if STABLE_TEST
				for (auto itr = m_aData.begin(); itr != m_aData.end(); ++itr)
				{
					if (itr->GetInvalid())
					{
						LogFormat("com %d is not valid", itr->GetID());
						continue;
					}
					event_call_itr = m_mEventMap.find(itr->GetID());
					if (event_call_itr != end)
					{
						event_call_itr->second(itr, itr->GetID(), itr->GetCatalog());
					}
				}
#endif

			}
		}
		void UnRegisterComponent(int componentId)
		{
			for (auto itr = m_aData.begin(); itr != m_aData.end(); itr++)
			{
				if (!itr->GetInvalid() && itr->GetID() == componentId)
				{
					ConstComponentEventMapItr end = m_mEventMap.end();
					ConstComponentEventMapItr removed = m_mEventMap.find(componentId);
					if (removed != end)
					{
						removed->second(nullptr, componentId, itr->GetCatalog());
					}
					m_aData.remove(itr);
#if STABLE_TEST
					for (auto itr1 = m_aData.begin(); itr1 != m_aData.end(); ++itr1)
					{
						if (itr1->GetInvalid())
						{
							LogFormat("com %d is not valid", itr1->GetID());
							continue;
						}
						const auto event_call_itr = m_mEventMap.find(itr1->GetID());
						if (event_call_itr != end)
						{
							event_call_itr->second(itr1, itr1->GetID(), itr1->GetCatalog());
						}
					}
#endif
					//remove is stable,no need to check dirty
					return;
				}
			}
		}
		bool operator == (const System& right) const
		{
			return this == &right;
		}
	protected:
		//array is not address stable structure,m_aData need a address stable structure
		//for example,remove or add an element will not change the addresses of other element
		//so need an array do not malloc memory when add,and will not move address when remove
		//if remove an element,just give this removed element a tag of not valid
		//if add new element,find if any invalid element exist,if so ,just do the copy construction
		//and for enough space,its needed to malloc enough space for this array
		StableArray<T> m_aData;
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
