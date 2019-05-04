#include "World.h"
#include "game/Time/GameTime.h"
#include "Logger/Logger.h"
#include "Server.h"
#include "game/Time/UpdateTime.h"
namespace server
{
	static World* instance = nullptr;
	World * World::GetInstance()
	{
		if (!instance)
		{
			instance = new World();
		}
		return instance;
	}

	void World::DestroyInstance()
	{
		safe_delete(instance);
	}

	World::World()
	{
		m_pEcs = new ecs::SystemContainer();
		m_pEcs->Initilize(ecs::SystemGroup::SERVER_WORLD);
		m_pEcs->AddSystem(ecs::SystemCatalog::INTERVAL_TIMER);
	}

	World::~World()
	{
		safe_delete(m_pEcs);
	}

	void World::Update(uint32_t diff)
	{
		time_t currentGameTime = GameTime::GetGameTime();

		//sWorldUpdateTime.UpdateWithDiff(diff);

		// Record update if recording set in log and diff is greater then minimum set in log
		//sWorldUpdateTime.RecordUpdateTime(GameTime::GetGameTimeMS(), diff, GetActiveSessionCount());

		/// <li> Handle session updates when the timer has passed
		//sWorldUpdateTime.RecordUpdateTimeReset();
		UpdateSessions(diff);
		m_pEcs->OnUpdate(diff);
		//sWorldUpdateTime.RecordUpdateTimeDuration("UpdateSessions");
	}
	void World::UpdateSessions(uint32 diff)
	{  ///- Add new sessions
		std::shared_ptr<WorldSession> sess = nullptr;
		while (addSessQueue.next(sess))
			AddSession_(sess);

		///- Then send an update signal to remaining ones
		for (SessionMap::iterator itr = m_mSessions.begin(), next; itr != m_mSessions.end(); itr = next)
		{
			next = itr;
			++next;

			///- and remove not active sessions from the list
			std::shared_ptr<WorldSession> pSession = itr->second;

			if (!pSession->Update(diff))    // As interval = 0
			{
				if (!RemoveQueuedPlayer(itr->second) && itr->second)
					m_disconnects[itr->second->GetSessionId()] = GameTime::GetGameTime();
				RemoveQueuedPlayer(pSession);
				m_mSessions.erase(itr);

			}
		}
	}
	std::shared_ptr<WorldSession> World::FindSession(uint32 id) const
	{
		SessionMap::const_iterator itr = m_mSessions.find(id);

		if (itr != m_mSessions.end())
			return itr->second;                                 // also can return nullptr for kicked session
		else
			return nullptr;
	}
	void World::AddSession(std::shared_ptr<WorldSession> s)
	{
		addSessQueue.add(s);
	}
	bool World::RemoveSession(uint32 id)
	{
		///- Find the session, kick the user, but we can't delete session at this moment to prevent iterator invalidation
		SessionMap::const_iterator itr = m_mSessions.find(id);

		if (itr != m_mSessions.end() && itr->second)
		{
			if (itr->second->PlayerLoading())
				return false;

			itr->second->KickPlayer();
		}

		return true;
	}
	void World::AddQueuedPlayer(std::shared_ptr<WorldSession> sess)
	{
		sess->SetInQueue(true);
		m_QueuedPlayer.push_back(sess);

		//TODO 
		//send queue msg to client
	}
	bool World::RemoveQueuedPlayer(std::shared_ptr<WorldSession> sess)
	{
		// sessions count including queued to remove (if removed_session set)
		uint32 sessions = GetActiveSessionCount();

		uint32 position = 1;
		Queue::iterator iter = m_QueuedPlayer.begin();

		// search to remove and count skipped positions
		bool found = false;

		for (; iter != m_QueuedPlayer.end(); ++iter, ++position)
		{
			if (*iter == sess)
			{
				sess->SetInQueue(false);
				sess->ResetTimeOutTime(false);
				iter = m_QueuedPlayer.erase(iter);
				found = true;                                   // removing queued session
				break;
			}
		}

		// iter point to next socked after removed or end()
		// position store position of removed socket and then new position next socket after removed

		// if session not queued then we need decrease sessions count
		if (!found && sessions)
			--sessions;

		// accept first in queue
		if ((!m_playerLimit || sessions < m_playerLimit) && !m_QueuedPlayer.empty())
		{
			std::shared_ptr<WorldSession> pop_sess = m_QueuedPlayer.front();
			pop_sess->InitializeSession();
			m_QueuedPlayer.pop_front();

			// update iter to point first queued socket or end() if queue is empty now
			iter = m_QueuedPlayer.begin();
			position = 1;
		}

		// update position from iter to end()
		// iter point to first not updated socket, position store new position
		for (; iter != m_QueuedPlayer.end(); ++iter, ++position)
			(*iter)->SendAuthWaitQue(position);

		return found;
	}
	void World::OnServerShutDown()
	{
		for (auto itr = m_mSessions.begin(); itr != m_mSessions.end(); itr++)
		{
			itr->second->KickPlayer();
		}
		m_disconnects.clear();
		m_QueuedPlayer.clear();
		m_mSessions.clear();
	}
	bool World::HasRecentlyDisconnected(std::shared_ptr<WorldSession> session)
	{
		if (!session)
			return false;

		for (DisconnectMap::iterator i = m_disconnects.begin(); i != m_disconnects.end();)
		{
			if (difftime(i->second, GameTime::GetGameTime()) < 1000)
			{
				if (i->first == session->GetSessionId())
					return true;
				++i;
			}
			else
				m_disconnects.erase(i++);
		}
		return false;
	}
	void World::UpdateMaxSessionCounters()
	{
		m_maxActiveSessionCount = sd_max(m_maxActiveSessionCount, uint32(m_mSessions.size() - m_QueuedPlayer.size()));
		m_maxQueuedSessionCount = sd_max(m_maxQueuedSessionCount, uint32(m_QueuedPlayer.size()));
	}
	void World::AddSession_(std::shared_ptr<WorldSession> s)
	{
		///- kick already loaded player with same account (if any) and remove session
		///- if player is in loading and want to load again, return
		if (!RemoveSession(s->GetSessionId()))
		{
			s->KickPlayer();                                    // session not added yet in session list, so not listed in queue
			return;
		}

		// decrease session counts only at not reconnection case
		bool decrease_session = true;

		// if session already exist, prepare to it deleting at next world update
		// NOTE - KickPlayer() should be called on "old" in RemoveSession()
		{
			SessionMap::const_iterator old = m_mSessions.find(s->GetSessionId());

			if (old != m_mSessions.end())
			{
				old->second->KickPlayer();
			}
		}

		m_mSessions[s->GetSessionId()] = s;
		//initilize ecs after lock queue unlocked,or may be fail in update of ecs system
		s->Initilize(m_pEcs);

		uint32 Sessions = GetActiveAndQueuedSessionCount();
		uint32 pLimit = GetPlayerAmountLimit();
		uint32 QueueSize = GetQueuedSessionCount(); //number of players in the queue

		//so we don't count the user trying to
		//login as a session and queue the socket that we are using
		if (decrease_session)
			--Sessions;

		if (pLimit > 0 && Sessions >= pLimit && !HasRecentlyDisconnected(s))
		{
			AddQueuedPlayer(s);
			UpdateMaxSessionCounters();
			LogFormat("PlayerQueue: Account id %u is in Queue Position (%u).", s->GetSessionId(), ++QueueSize);
			return;
		}

		s->InitializeSession();

		UpdateMaxSessionCounters();

		// Updates the population
		if (pLimit > 0)
		{
			float popu = (float)GetActiveSessionCount();              // updated number of users on the server
			popu /= pLimit;
			popu *= 2;
		}
	}
}