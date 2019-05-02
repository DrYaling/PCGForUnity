#ifndef I_component_h
#define I_component_h
#include "define.h"
namespace ecs
{
	enum class ComponentCatalog :uint16_t
	{
		MOVEMENT = 0,
		STATUS,
		INTERVAL_TIMER,
	};
	class IEntity;
	class ISystem;
	/*
	非线程安全ECS数据接口，只提供几个获取属性的接口
	*/
	class IComponent
	{
		friend ISystem;
		friend IEntity;
	public:
		explicit IComponent(int32_t id) :m_ID(id), flags(1) {}
		explicit IComponent(const IComponent& copy) :m_ID(copy.m_ID), flags(copy.flags) {}
		~IComponent() {}
		inline uint32_t GetID() { return m_ID; }
		inline bool IsDirty() { return flags & 0x1; }
		inline ComponentCatalog GetCatalog()
		{
			return (ComponentCatalog)((flags >> 16) & 0x0000ffff);
		}
		bool operator == (const IComponent& right)
		{
			return m_ID == right.m_ID;
		}
		IComponent() = delete;
	private:
	protected:
		inline void SetDirty(bool dirty)
		{
			if (dirty)
				flags |= 1;
			else
				flags &= 0xffff0000;
		}
		inline void SetCatalog(ComponentCatalog catalog)
		{
			flags |= ((uint16_t)catalog << 16) & 0xffff0000;
		}
	private:
		uint32_t m_ID;
		uint32_t flags;
	};
}
#endif
