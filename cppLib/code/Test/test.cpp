#include "Network/Socket/SocketServer.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>
void StartTestServer()
{
	SocketServer server(SocketType::SOCKET_UDP);
	server.SetAddress("127.0.0.1", 8081);
	bool bret = server.StartUp();
	LogFormat("StartUp ret %d\n", bret);
	//server.SetBlock(true);
	char buff[1024];
	while (true)
	{
		sleep(10);
		memset(buff, 0, sizeof(buff));
		/*auto ret = server.Recv(buff, sizeof(buff));
		if (ret.nresult == 0)
		{
			printf_s("Recv ret %d,%d\n", ret.nresult, ret.nbytes);
		}*/
	}
}
int main()
{
	//LogFormat("Test.main");
	std::thread thr(StartTestServer);
	thr.detach();
	sleep(4000);
	
	char c;
	std::cin >> c;
	return 0;
}