#include "GameEngine_Bindings.h"
#include "Network/Socket/SocketServer.h"
#include "server/Server.h"
#include "Logger/Logger.h"
#include "Threading/ThreadManager.h"
using namespace server;
using namespace logger;
#include "Client/Client.h"
#include <memory>
std::shared_ptr<client::Client> _client;
EXPORT_API void STD_CALL StartUpClientTest()
{
	LogFormat("StartUpClientTest");
	//_client = std::make_shared<client::Client>();
	//sThreadManager->AddTask(threading::ThreadTask(std::bind(&client::Client::Run, _client)));
}

void StartUpServer(const char* ip, int32_t port)
{
	LogFormat("StartUpServer");
	//sSocketServer->SetAddress(ip, port);
	//threading::ThreadManager::SetThreadCount(10);

	/*sThreadManager->AddTask(threading::ThreadTask(std::bind([]
	{
		sServer->MainLoop();
	})));*/
}

void StopServer()
{
	LogFormat("stop server and client");
	//sServer->ShutDown();
	//_client->Stop();
	//_client = nullptr;
}
