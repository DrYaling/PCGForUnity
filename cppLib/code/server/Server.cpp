#include "Server.h"
#include "Utinities/Timer.h"
#include "World/World.h"
#include <thread>
#include "game/Time/GameTime.h"
#include "Network/Socket/SocketServer.h"
#include "Logger/Logger.h"
#include "game\Time\UpdateTime.h"
namespace server
{
	static std::atomic<uint32> m_worldLoopCounter = 0;
	Server::Server() :
		m_eState(ServerState::NORMAL),
		m_eExitCode(ServerExitCode::NORMAL),
		m_ShutdownTimer(0),
		m_ShutdownMask(0)
	{
		m_worldLoopCounter = 0;
	}


	Server::~Server()
	{
	}
	void Server::Reset()
	{
		m_worldLoopCounter = 0;
		m_eState = ServerState::NORMAL;
		m_eExitCode = ServerExitCode::NORMAL;
	}
	int Server::MainLoop()
	{
		uint32 realCurrTime;
		uint32 realPrevTime = getMSTime();
		std::shared_ptr<SocketServer> pSocketServer = sSocketServer;
		pSocketServer->SetMTU(512);
		pSocketServer->SetServerId(0);
		pSocketServer->SetAcceptSessionHandle([](std::shared_ptr<KcpSession> session, uint32_t id)->void
		{
			sWorld->AddSession(std::make_shared<WorldSession>(id, "", session));
		});
		const bool bret = pSocketServer->StartUp();
		if (!bret)
		{
			return -1;
		}
		LogFormat("StartUp ret %d\n", bret);
		sWorldUpdateTime.SetRecordUpdateTimeInterval(15000);
		while (!IsStopped())
		{
			++m_worldLoopCounter;

			realCurrTime = getMSTime();

			uint32 diff = getMSTimeDiff(realPrevTime, realCurrTime);
			_UpdateGameTime();
			pSocketServer->Update(diff);
			sWorld->Update(diff);
			sWorldUpdateTime.UpdateWithDiff(diff);
			sWorldUpdateTime.RecordUpdateTime(GameTime::GetGameTimeMS(), diff, sWorld->GetActiveSessionCount());
			realPrevTime = realCurrTime;

			uint32 executionTimeDiff = getMSTimeDiff(realCurrTime, getMSTime());
			// we know exactly how long it took to update the world, if the update took less than WORLD_SLEEP_CONST, sleep for WORLD_SLEEP_CONST - world update time
			if (executionTimeDiff < WORLD_SLEEP_CONST)
				std::this_thread::sleep_for(std::chrono::milliseconds(WORLD_SLEEP_CONST - executionTimeDiff));
			if (m_eState == ServerState::DELAYED_SHUTDOWN)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				ShutdownServ(0, 0, 0, "");
				ShutDown();
			}
			else if (m_eState == ServerState::SHUTDOWN)
			{
				ShutdownServ(0, 0, 0, "");
			}

		}
		try
		{
			SocketServer::Destroy();
		}
		catch (const std::exception& e)
		{
		}
		LogFormat("server exited with code %d", m_eExitCode);
		return static_cast<int>(m_eExitCode);
	}
	void Server::ShutDown()
	{
		SetExitCode(ServerExitCode::NORMAL);
		SetServerState(ServerState::SHUTDOWN);
	}
	void Server::ShutdownServ(uint32 time, uint32 options, uint8 exitcode, const std::string & reason)
	{ // ignore if server shutdown at next tick
		if (IsStopped())
			return;

		m_ShutdownMask = options;
		m_eExitCode = (ServerExitCode)exitcode;

		///- If the shutdown time is 0, set m_stopEvent (except if shutdown is 'idle' with remaining sessions)
		if (time == 0)
		{
			if (true || !(options & (uint32)ShutdownMask::SHUTDOWN_MASK_IDLE) || sWorld->GetActiveAndQueuedSessionCount() == 0)
				sWorld->OnServerShutDown();                         // exist code already set
			else
				m_ShutdownTimer = 1;                            //So that the session count is re-evaluated at next world tick
		}
		///- Else set the shutdown timer and warn users
		else
		{
			m_ShutdownTimer = time;
			//todo send shutdown msg to clients
		}

	}
	uint32_t Server::GetWorldFrameCounter()
	{
		return m_worldLoopCounter;
	}
	void Server::_UpdateGameTime()
	{
		///- update the time
		time_t lastGameTime = GameTime::GetGameTime();
		GameTime::UpdateGameTimers();

		uint32 elapsed = uint32(GameTime::GetGameTime() - lastGameTime);

		///- if there is a shutdown timer
		if (!IsStopped() && m_ShutdownTimer > 0 && elapsed > 0)
		{
			///- ... and it is overdue, stop the world (set m_stopEvent)
			if (m_ShutdownTimer <= elapsed)
			{
				SetServerState(ServerState::DELAYED_SHUTDOWN);
				if (!(m_ShutdownMask & (uint32)ShutdownMask::SHUTDOWN_MASK_IDLE) || sWorld->GetActiveAndQueuedSessionCount() == 0)
					SetServerState(ServerState::DELAYED_SHUTDOWN);                         // exist code already set
				else
					m_ShutdownTimer = 1;                        // minimum timer value to wait idle state
			}
			///- ... else decrease it and if necessary display a shutdown countdown to the users
			else
			{
				m_ShutdownTimer -= elapsed;
			}
		}
	}
}