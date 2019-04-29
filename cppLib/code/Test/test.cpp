#include "Network/Socket/SocketServer.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>
#include "Network/Socket/SocketTime.h"
/*
#include "Generators/TerrianGenerator/Mountain.h"
#include "Generators/TerrianGenerator/Diamond_Square.h"*/
#include "generator.h"
using namespace generator;
class IComponent
{
public:
	IComponent() :d0(0), d1(0) {}
	~IComponent() {}

public:
	char d0;
	char d1;
};
class Component :IComponent
{
public:
	Component();
	~Component();
	void Print() 
	{
		LogFormat("comp addr 0-3  %d,%d,%d,%d", &d0, &d1, &d2, &d3);
		LogFormat("comp addr this %d", this);
	}

public:
	char d2;
	char d3;
};

Component::Component() :d2(0), d3(0)
{
}

Component::~Component()
{
}
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
	Component comp;
	comp.Print();
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
/*
	sleep(3000);
	//LogFormat("Test.main");
	std::thread thr(StartTestServer);
	thr.detach();
	sleep(4000);
*/
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