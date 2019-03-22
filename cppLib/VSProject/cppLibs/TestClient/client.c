#include "Network/Socket/CSocket.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>

int main()
{
	sleep(2000);
	Socket socket(SocketType::SOCKET_TCP);
	socket.SetAddress("127.0.0.1", 8081);
	socket.SetSocketMode(SocketSyncMode::SOCKET_ASYNC);
	auto callback = [&socket](int size, char* buff)->void {
		LogFormat("recv server msg len :%d,content:%s", size, buff);
		socket.SetRecvCallback(nullptr);
	};
	socket.SetRecvCallback(callback);
	while (!socket.Connected())
	{
		//socket.Send(NULL, 0);
		socket.Connect();
		sleep(1000);
		SkyDream::ListConn l;
	}
	char buff[10] = { "12312312" };
	int i = 5;
	Log("connect to 127.0.0.1:8081 success");
	while (i-- > 0)
	{
		auto t = socket.TrySend(buff, sizeof(buff), 0);
		LogFormat("send ret %d,%d", t.nresult, t.nbytes);
		sleep(3276);
	}
	socket.Close();
	return 0;
}