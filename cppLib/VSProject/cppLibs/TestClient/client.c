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
#include "game\Time\GameTime.h"
#include "game\Time\UpdateTime.h"
using namespace logger;
int main()
{
	//sleep(100);
	int client_count = 500;
	std::vector<KcpClient*> clients;
	char buff[480] = { "9" };
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

	std::vector<uint32_t> _timer(clients.size());
	for (int i = 0; i < clients.size(); i++)
	{
		_timer[i] = 0;
	}
	std::vector<uint32_t> _interval(clients.size());
	for (int i = 0; i < clients.size(); i++)
	{
		_interval[i] = _irandom(10, 50) * 100;
	}
	sWorldUpdateTime.SetRecordUpdateTimeInterval(15000);
	while (true)
	{
		sleep(15);
		GameTime::UpdateGameTimers();
		time_t currentGameTime = GameTime::GetGameTime();
		uint32_t currentGameTimeMS = GameTime::GetGameTimeMS();

		sWorldUpdateTime.UpdateWithDiff(15);

		int	 count = 0;
		for (auto itr = clients.begin(); itr != clients.end(); itr++)
		{
			if ((*itr)->IsAlive())
				count++;
		}
		// Record update if recording set in log and diff is greater then minimum set in log
		sWorldUpdateTime.RecordUpdateTime(GameTime::GetGameTimeMS(), 15, count);
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
			ProfilerStart(G3D::format("client %u update ",(*itr)->GetClientId()).c_str());
			(*itr)->Update(15);
			ProfilerEnd(5);
		}
	}
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