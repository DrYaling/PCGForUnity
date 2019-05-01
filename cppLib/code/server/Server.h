#ifndef _server_h
#define  _server_h
#include "define.h"
#include <atomic>
#include <string>
#include "game/Time/GameTime.h"
namespace server
{
#define WORLD_SLEEP_CONST 10
	enum class ServerState
	{
		NORMAL = -1,
		SHUTDOWN,
		DELAYED_SHUTDOWN,
	};
	enum class ServerExitCode
	{
		NORMAL = 0,
	};
	enum class ShutdownMask : uint32
	{
		SHUTDOWN_MASK_RESTART = 1,
		SHUTDOWN_MASK_IDLE = 2,
		SHUTDOWN_MASK_FORCE = 4
	};

	class Server;
#define sServer Server::GetInstance()
	class Server
	{
	public:
		Server();
		~Server();
		static Server* GetInstance()
		{
			static Server instance;
			return &instance;
		}
		void Reset();
		int MainLoop();
		bool IsStopped()
		{
			return m_eState == ServerState::SHUTDOWN;
		}
		void ShutDown();
		void SetExitCode(ServerExitCode code)
		{
			m_eExitCode = code;
		}
		void ShutdownServ(uint32 time, uint32 options, uint8 exitcode, const std::string& reason = std::string());
		uint32_t GetWorldFrameCounter();
		void ShutDownAfter(uint32_t seconds) { m_ShutdownTimer = GameTime::GetGameTimeMS()/1000 + seconds; }
	private:
		void SetServerState(ServerState state)
		{
			m_eState = state;
		}
		void _UpdateGameTime();
	private:
		ServerState m_eState;
		ServerExitCode m_eExitCode;
		uint32_t m_ShutdownTimer;
		uint32 m_ShutdownMask;
	};
#define  sServer Server::GetInstance()
}
#endif