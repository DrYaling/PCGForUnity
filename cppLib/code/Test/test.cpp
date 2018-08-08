#include "Network/Socket/CSocket.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
int main()
{
	//LogFormat("Test.main");
	Socket socket(SocketType::SOCKET_TCP);
	//socket.Send(NULL, 0);
	socket.Connect("127.0.0.1", 8081);
	SkyDream::ListConn l;
	char buff[10] = { "12312312" };
	auto t = socket.Send(buff, sizeof(buff));
	printf_s("send ret %d,%d", t.nresult, t.nbytes);
	char c;
	std::cin >> c;
	return 0;
}