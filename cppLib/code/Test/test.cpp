#include "Network/Socket/SocketServer.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>
#include "Network/Socket/SocketTime.h"
#include "ECS/SystemContainer.h"
#include "generator.h"
#include "ECS/StatusComponent.h"
#include "ECS/MovementComponent.h"
#include "ECS/EntityDemo.h"
#include "server/Server.h"
#include "Logger/leakHelper.h"
using namespace generator;
using namespace ecs;
void StartTestServer()
{
	SocketServer* Server = sSocketServer;
	Server->SetMTU(512);
	Server->SetAddress("127.0.0.1", 8081);
	bool bret = Server->StartUp();
	LogFormat("StartUp ret %d\n", bret);
	//Server->(true);
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
			sSocketServer->Update(10);
		}
	}
	delete sSocketServer;
}
using namespace server;
int ServerWorker()
{
	sServer->ShutDownAfter(6);
	return sServer->MainLoop();
}
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
//	new int[20];
	SystemContainer* pc = new SystemContainer();
	pc->Initilize(SystemGroup::SERVER_WORLD);
	pc->OnUpdate(1);
	LogFormat("\t\n");
	pc->AddSystem(SystemCatalog::MOVEMENT);
	pc->OnUpdate(2);
	LogFormat("\t\n");
	pc->AddSystem(SystemCatalog::STATUS);
	pc->OnUpdate(3);
	LogFormat("");
	pc->SetPriority(SystemCatalog::STATUS, 1);
	pc->OnUpdate(3);
	LogFormat("");
	pc->SetPriority(SystemCatalog::MOVEMENT, 2);
	pc->OnUpdate(3);
	LogFormat("");
	pc->SetPriority(SystemCatalog::MOVEMENT, 0);
	pc->OnUpdate(3);
	LogFormat("");
	EntityDemo* pDemo = new EntityDemo();
	pDemo->Initilize(pc);
	pc->OnUpdate(4);
	LogFormat("");
	pDemo->ReleaseTest();
	pc->OnUpdate(5);
	LogFormat("");

	pDemo->ChangeComponentDirty(1, ComponentCatalog::MOVEMENT, true);
	pDemo->ChangeComponentDirty(2, ComponentCatalog::STATUS, true);
	pDemo->ChangeComponentDirty(4, ComponentCatalog::STATUS, true);
	pDemo->ChangeComponentDirty(6, ComponentCatalog::STATUS, true);
	pDemo->ChangeComponentDirty(3, ComponentCatalog::STATUS, true);
	pc->OnUpdate(5);
	LogFormat("");

	safe_delete(pDemo);
	safe_delete(pc);
	DWORD start, stop;
	start = GetTickCount();
	/*auto p = Vector3();
	p.x = 2370;
	p.y = 5428;
	p.z = 7562;
	MountainGen gen = MountainGen(std::move(p), 4);
	gen.Start();*/
	/*Diamond_Square ds(2, 1, 40.0f);
	ds.SetProcessHandler([](int32_t process)->void {
		LogFormat("progress %d", process);
	});
	float h[4] = { 10,100,50,41 };
	ds.Start(h);
	/ *while (!ds.IsFinished())
	{
		sleep(100);
	}
	LogFormat("Diamond_Square finished");* /
	stop = GetTickCount();
	LogFormat("Diamond_Square caculate time %d ms", stop - start);
	std::vector<Vector3> v3[3];
	std::vector<Vector3> normals[3];
	std::vector<int32_t> idx[3];
	ds.GenerateTerrian(idx, v3, normals, 10);
	stop = GetTickCount();
	LogFormat("Diamond_Square total gen time %d ms", stop - start);*/
	//while (1) sleep(1000);

	//sleep(3000);
	//LogFormat("Test.main");
	/*std::thread thr(ServerWorker());
	thr.detach();*/
	auto ret = ServerWorker();
	//new int[20];
	_CrtDumpMemoryLeaks();
	LogFormat("ServerWorker ret %d", ret);
	return ret;
	sleep(4000);

	G3D::Vector3 p(0.1f, 0.f, 1.2f);
	for (size_t i = 0; i < 3; i++)
	{
		//LogFormat("%d", (uint32&)p[i]);
	}
	std::vector<int> list;
	list.resize(100);
	list.clear();
	//LogFormat("list size %d,%d", list.size(), list.capacity());
	{
		std::vector<int> tmp = list;
		tmp.swap(list);
	}
	//LogFormat("list size %d,%d", list.size(), list.capacity());
	while (true)
	{
		sleep(1000);
	}
	char c;
	std::cin >> c;
	return 0;
}