#ifndef _World_Session_h
#define _World_Session_h
#include "define.h"
#include "Network/Socket/KcpSession.h"
#include "game/Time/GameTime.h"
#include <atomic>
#include <memory>
#include "Network/Socket/MessageBuffer.h"
#include "ECS/IEntity.h"
#include "ECS/TimeComponents.h"
namespace server
{
	//with no public,when use shared_from_this throw bad_weak_ptr exception
	class WorldSession :public std::enable_shared_from_this<WorldSession>, public ecs::IEntity
	{
	public:
		WorldSession(uint32 id, std::string&& name, std::shared_ptr<KcpSession> sock);
		WorldSession(const WorldSession&) = delete;
		WorldSession(const WorldSession&&) = delete;
		~WorldSession();
		void KickPlayer();
		/// Session in auth.queue currently
		void SetInQueue(bool state) { m_bInQueue = state; }
		bool GetInQueue() const { return m_bInQueue; }
		uint32_t GetSessionId() { return m_nSessionId; }
		bool PlayerLoading() const { return false; }
		void ResetTimeOutTime(bool state) { m_nTimeOutTime = GameTime::GetGameTime() + 30000; }
		void InitializeSession();
		void SendAuthWaitQue(uint32 position);
		bool Update(uint32_t diff);
		bool IsConnectionIdle() const
		{
			return m_nTimeOutTime < GameTime::GetGameTime() && !m_bInQueue;
		}
	public:
		//ecs

		void Initilize(ecs::SystemContainer* pContainer) override;
		void OnComponentChangeEvent(ecs::IComponent* com, uint32_t comId, ecs::ComponentCatalog catalog) override;
	private:
		bool OnReceivePacket(int, uint8*, int);
	private:
		std::shared_ptr<KcpSession> m_pSocket;
		uint32_t m_nSessionId;
		std::atomic<time_t> m_nTimeOutTime;
		bool m_bInQueue;
		ecs::IntervalTimer* m_pTimer;
		uint32_t m_nTimerId;
		ecs::IntervalTimer* m_pHeatBeatTimer;
		uint32_t m_nHeartBeatTimerId;
	};
}
#endif
