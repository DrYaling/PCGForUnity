#ifndef GameEngine_client_h
#define  GameEngine_client_h
#include "define.h"
#include <atomic>
namespace client
{
	class Client
	{
	public:
		Client();
		~Client();
		void Run();
		void Stop();
	private:
		std::atomic_bool m_bStopped;
	};
}
#endif