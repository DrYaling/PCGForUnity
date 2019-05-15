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
		inline uint32_t GetID() const { return m_ID; }
		inline bool IsDirty() const { return flags & 0x1; }
		inline ComponentCatalog GetCatalog() const
		{
			return static_cast<ComponentCatalog>((flags >> 16) & 0x0000ffff);
		}
		void SetInvalid(bool invalid)
		{
			if (invalid)
				flags |= 0x2;
			else
				flags &= 0xfffffffd;
		}
		bool GetInvalid() const
		{
			return flags & 0x00000002;
		}
		bool operator == (const IComponent& right) const
		{
			return m_ID == right.m_ID;
		}
		IComponent() = delete;
	protected:
		inline void SetDirty(bool dirty)
		{
			if (dirty)
				flags |= 1;
			else
				flags &= 0xfffffffe;
		}
		inline void SetCatalog(ComponentCatalog catalog)
		{
			flags |= (static_cast<uint16_t>(catalog) << 16) & 0xffff0000;
		}
	private:
		uint32_t m_ID;
		uint32_t flags;//0xffff0000 catalog,bit 0 dirty,bit 1 invalid
	};
}
#endif
