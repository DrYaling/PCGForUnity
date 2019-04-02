#include "Network/Socket/SocketServer.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>
#include "Network/Socket/SocketTime.h"
#include "Generators/TerrianGenerator/Mountain.h"
using namespace generator;

void StartTestServer()
{
	SocketServer* server = sSocketServer;
	server->SetMTU(512);
	server->SetAddress("127.0.0.1", 8081);
	bool bret = server->StartUp();
	LogFormat("StartUp ret %d\n", bret);
	//server->(true);
	char buff[1024];
	SocketTime.timeStampSinceStartUp = 0;
	std::mutex _mtx;
	while (true)
	{
		sleep(10);
		SocketTime.Update(10);
		{
			std::lock_guard<std::mutex> lck(_mtx);
			//LogFormat("Socket time %d", SocketTime.timeStampSinceStartUp);
			sSocketServer->Update();
		}
	}
	delete sSocketServer;
}
int main()
{
	/*DWORD start, stop;
	start = GetTickCount();
	auto p = Vector3();
	p.x = 2370;
	p.y = 5428;
	p.z = 7562;
	MountainGen gen = MountainGen(std::move(p), 4);
	gen.Start();
	stop = GetTickCount();
	LogFormat("MountainGen gen time %d ms", stop - start);*/
	sleep(3000);
	//LogFormat("Test.main");
	std::thread thr(StartTestServer);
	thr.detach();
	sleep(4000);
	
	char c;
	std::cin >> c;
	SocketTime_t::Destroy();
	return 0;
}