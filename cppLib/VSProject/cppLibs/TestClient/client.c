#include "Network/Socket/CSocket.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>
int main()
{
	//LogFormat("sockaddr_in size %d", sizeof(SockAddr_s));
	sleep(2000);
	Socket socket(SocketType::SOCKET_UDP);
	socket.SetAddress("127.0.0.1", 8081);
	socket.SetSocketMode(SocketSyncMode::SOCKET_ASYNC);
	auto callback = [&socket](int size, char* buff)->void {
		LogFormat("recv server msg len :%d,content:%s,sockaddr_in size %d", size, buff,sizeof(sockaddr_in));
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
	char buff[] = { "12312312" };
	char data[255] = {};
	int i = 9;
	Log("connect to 127.0.0.1:8081 success");
	while (i-- > 0)
	{
		PacketHeader header;
		header.Size = sizeof(buff);
		header.Command = 1;
		memcpy(data, &header, sizeof(header));
		sprintf_s(data + sizeof(header),255, "%s", buff);
		auto t = socket.TrySend(data,sizeof(header)+ sizeof(buff), 0);
		LogFormat("send ret %d,%d-buff size %d", t.nresult, t.nbytes,sizeof(buff));
		sleep(3276);
	}
	socket.Close();
	return 0;
}