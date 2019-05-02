#ifndef _World_h
#define _World_h
#include "define.h"
#include "WorldSession/WorldSession.h"
#include <unordered_map>
#include <queue>
#include "Utinities/LockedQueue.h"
#include "ECS/SystemContainer.h"
namespace server
{
	class World
	{
		typedef std::unordered_map<uint32, std::shared_ptr<WorldSession>> SessionMap;
		//Player Queue
		typedef std::list<std::shared_ptr<WorldSession>> Queue;

	public:
		static World* GetInstance();
		static void DestroyInstance();
		World();
		~World();
		void Update(uint32_t time_diff);
		void UpdateSessions(uint32 diff);
		std::shared_ptr<WorldSession> FindSession(uint32 id) const;
		void AddSession(std::shared_ptr<WorldSession> s);
		bool RemoveSession(uint32 id);
		void AddQueuedPlayer(std::shared_ptr<WorldSession>);
		bool RemoveQueuedPlayer(std::shared_ptr<WorldSession> session);
		SessionMap const& GetAllSessions() const { return m_mSessions; }
		uint32 GetActiveAndQueuedSessionCount() const { return m_mSessions.size(); }
		uint32 GetActiveSessionCount() const { return m_mSessions.size() - m_QueuedPlayer.size(); }
		uint32 GetQueuedSessionCount() const { return m_QueuedPlayer.size(); }
		uint32 GetPlayerAmountLimit() const { return m_playerLimit; }
		void OnServerShutDown();
	private:
		bool HasRecentlyDisconnected(std::shared_ptr<WorldSession>);
		void UpdateMaxSessionCounters();
	private:
		SessionMap m_mSessions;
		typedef std::unordered_map<uint32, time_t> DisconnectMap;
		DisconnectMap m_disconnects;
		// sessions that are added async
		void AddSession_(std::shared_ptr<WorldSession> s);
		LockedQueue<std::shared_ptr<WorldSession>> addSessQueue;
		Queue m_QueuedPlayer;
		uint32_t m_playerLimit;
		uint32 m_maxActiveSessionCount;
		uint32 m_maxQueuedSessionCount;
		uint32 m_PlayerCount;
		ecs::SystemContainer* m_pEcs;

	};
#define sWorld World::GetInstance()
}
#endif
