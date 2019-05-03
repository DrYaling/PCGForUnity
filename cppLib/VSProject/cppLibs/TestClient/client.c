#include "Network/Socket/CSocket.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>
#include "Network/Socket/KcpClient.h"
#include "Network/Socket/SocketTime.h"
#include "Generator/generator.h"
int main()
{
	//sleep(100);
	int client_count = 150;
	std::vector<KcpClient*> clients;
	char buff[345] = { "9" };
	char data[2550] = { 5 };
	PacketHeader header;
	header.Size = sizeof(buff);
	header.Command = 7;
	memcpy(data, &header, sizeof(header));
	//header.Size = _irandom(15, sizeof(buff));
	sprintf_s(data + sizeof(header), header.Size, "%s", buff);
	while (client_count--)
	{
		KcpClient* client = new KcpClient(0);
		clients.push_back(client);
		SocketTime.timeStampSinceStartUp = 0;
		client->SetAddress("127.0.0.1", 8081);
		auto callback = [&client](int cmd, const uint8* buff, int size)->bool {
			LogFormat("client %u recv server msg cmd :%d,content:%s,msg size %d", client->GetClientId(), cmd, buff, size);
			//socket.SetRecvCallback(nullptr);
			return true;
		};
		client->SetReceiveCallBack(callback);
		client->SetConnectTimeout(0);
		client->Connect();
	}
	std::thread t([&]()->void {
		std::vector<uint32_t> _timer(clients.size());
		for (int i = 0; i < clients.size(); i++)
		{
			_timer[i] = 0;
		}
		std::vector<uint32_t> _interval(clients.size());
		for (int i = 0; i < clients.size(); i++)
		{
			_interval[i] = _irandom(1, 10) * 100;
		}
		while (true)
		{
			sleep(15);

			for (int i = 0; i < clients.size(); i++)
			{
				_timer[i] += 15;
				if (_timer[i] > _interval[i])
				{
					_timer[i] = 0;
					if (clients[i]->IsAlive())
					{
						clients[i]->Send(data, sizeof(header) + header.Size);
					}
				}
			}
			for (auto itr = clients.begin(); itr != clients.end(); itr++)
			{
				(*itr)->Update(15);
			}
		}
	});
	t.detach();
	LogFormat("clients idle");
	while (true)
	{
		sleep(100000);
	}

	//client.Close();
	//SocketTime_t::Destroy();
	while (true)
	{
		sleep(1);
	}
	return 0;
}