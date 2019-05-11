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
#include "Utinities/safe_array.h"
#include "Utinities/timer.h"
#include <array>
#include "Threading/ThreadPool.h"
using namespace generator;
using namespace ecs;
namespace transformMap
{
	std::string GetEngineDir()
	{
		return "";
	}
}



template <class F, class ... Args>
class templateFunction
{
public:
	templateFunction() = delete;
	explicit templateFunction(F&& function, Args&&... args)
	{
		auto func = std::bind(std::forward<F>(function), std::forward<Args>(args)...);
	}
	//std::shared_ptr < std::binf> func;
};
#include "Threading/ThreadManager.h"
#include "server/WorldSession/WorldSession.h"
int	 test_threadPool()
{
	using namespace threading;
	ThreadPool pool(4);
	auto lmd = [](int inp)->void
	{
		LogFormat("lmd input %d", inp);
	};
	using namespace server;
	//测试绑定对象生命周期
	std::function<void()> bsession;
	{
		std::shared_ptr<KcpSession> kcp = std::make_shared<KcpSession>(0, std::move(SockAddr_t()));
		bsession = std::bind(&WorldSession::Initilize, std::make_shared<WorldSession>(0, "", kcp), nullptr);
	}
	sThreadManager->AddTask(ThreadTask(bsession));
	std::function<void()> f_v_0 = std::bind(lmd, 2);
	sThreadManager->AddTask(ThreadTask(f_v_0));
	threading::ThreadManager::GetInstance()->AddTask(threading::ThreadTask(std::bind([](std::string str)
	{
		LogFormat("str is %s", str.c_str());
	}, "hahaha")));
	ThreadManager::Destroy();
	/*std::vector< std::future<int> > results;

	for (int i = 0; i < 8; ++i) {
		std::function<int(void)> call = [i] {
			std::cout << "hello " << i << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "world " << i << std::endl;
			return i * i;
		};
		auto func = [i] {
			std::cout << "hello " << i << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "world " << i << std::endl;
			return i * i;
		};
		results.emplace_back(
			pool.enqueue(call)
		);
	}
	LogFormat("thread pool test geting result");
	int idx = 0;
	for (auto && result : results)
		LogFormat("result %d is %d", idx++,result.get());
	std::cout << std::endl;*/
	return 0;
}

int	Safe_Array_Test()
{
	LARGE_INTEGER start = { 0 };
	LARGE_INTEGER end = { 0 };
#define  _size 2500*2500
	float* darray = new float[_size];
	safe_array<float> sarray(_size);
	float *_array = new float[_size];
	std::array < float, (_size > 100000) ? 100000 : _size > std_array{};
	std_array.size();
	//float_array_fixed<_size> farray;
	std::vector<float> vect(10);
	vect[0] = 101;
	const float t1 = vect[0];
	sarray[0] = 102;
	const float t2 = sarray[0];
	//farray[0] = 103;
	//const float t3 = farray[0];
	QueryPerformanceFrequency(&start);
	LogFormat("t1 %f,t2 %f,t3 %f,t %lld", t1, t2, 0, start.QuadPart);
	sarray.Print();
	//	farray.Print();
		//auto begin = getMSTime();
	QueryPerformanceCounter(&start);
	for (int i = 0; i < _size; i++)
	{
		sarray[i] = 0.01f;
	}
	QueryPerformanceCounter(&end);
	LogFormat("sarray cost %lld us,size %d", end.QuadPart - start.QuadPart, _size);
	start = end;
	for (int i = 0; i < _size; i++)
	{
		_array[i] = 0.01f;
	}
	/*QueryPerformanceCounter(&end);
	LogFormat("array cost %lld us,size %d", end.QuadPart - start.QuadPart, _size);
	start = end;
	for (int i = 0; i < _size; i++)
	{
		farray[i] = 0.01f;
	}
	QueryPerformanceCounter(&end);
	LogFormat("farray cost %lld us,size %d", end.QuadPart - start.QuadPart, _size);
	start = end;
	float* ptr = farray.data();
	for (int i = 0; i < _size; i++)
	{
		ptr[i] = 0.01f;
	}*/
	QueryPerformanceCounter(&end);
	LogFormat("farray data cost %lld us,size %d", end.QuadPart - start.QuadPart, _size);
	start = end;
	for (int i = 0; i < 2500; i++)
	{
		for (int j = 0; j < 2500; j++)
		{
			darray[i + j * 2500] = 0.01f;
		}
	}
	QueryPerformanceCounter(&end);
	LogFormat("darray data cost %lld us,size %d", end.QuadPart - start.QuadPart, _size);
	start = end;
	for (int i = 0; i < _size && i < std_array.size(); i++)
	{
		std_array[i] = 0.01f;
	}
	QueryPerformanceCounter(&end);
	LogFormat("std_array cost %lld us,size %d", end.QuadPart - start.QuadPart, std_array.size());

	return 0;
}
int StartTestServer()
{
	SocketServer* Server = sSocketServer;
	Server->SetMTU(512);
	Server->SetAddress(/*"127.0.0.1"*/nullptr, 8081);
	const bool bret = Server->StartUp();
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
	return 0;
}
using namespace server;
int ServerWorker()
{
	sServer->ShutDownAfter(6);
	return sServer->MainLoop();
}
#include "Bridge/MeshBindings.h"
void GenerateTest()
{

}
void ECS_Test()
{
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
}
int main()
{
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	auto ret = ServerWorker();
	threading::ThreadManager::Destroy();
	//new int[20];
	_CrtDumpMemoryLeaks();
	LogFormat("ServerWorker ret %d", ret);
	char c = 0;
	scanf_s("%c", &c);
	return ret;
}