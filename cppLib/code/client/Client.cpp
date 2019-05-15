#include "Client.h"
#include "Network/Socket/KcpClient.h"
#include <Logger/Logger.h>
#include "game/Time/GameTime.h"
#include "game/Time/UpdateTime.h"
#include "Generator/generator.h"
#include "Threading/ThreadManager.h"
#include <stdio.h>
using namespace logger;
namespace client
{
	Client::Client():
		m_bStopped(false)
	{
	}


	Client::~Client()
	{
		LogFormat("client destroyed");
	}

	void Client::Run()
	{
		LogFormat("client Run");
		if (m_bStopped.load(std::memory_order_relaxed))
			return;
		//sleep(100);
		int client_count = 1;
		std::vector<std::shared_ptr<KcpClient>> clients;
		char buff[480] = { "9" };
		char data[2550] = { 5 };
		PacketHeader header;
		header.Size = sizeof(buff);
		header.Command = 7;
		memcpy(data, &header, sizeof(header));
		//header.Size = _irandom(15, sizeof(buff));
		sprintf_s(data + sizeof(header), header.Size, "%s", buff);
		while (client_count-- && !m_bStopped.load(std::memory_order_relaxed))
		{
			std::shared_ptr<KcpClient> client = std::make_shared<KcpClient>(0);
			clients.push_back(client);
			client->SetAddress("127.0.0.1", 8081);
			const auto callback = [&client](int cmd, const uint8* buff, int size)->bool {
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
		while (!m_bStopped.load(std::memory_order_relaxed))
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
				char buff[35];
				sprintf_s(buff, "client %u update ", (*itr)->GetClientId());
				logger::ProfilerStart(buff);
				(*itr)->Update(15);
				logger::ProfilerEnd(5);
			}
		}
	}

	void Client::Stop()
	{
		m_bStopped = true;
		LogFormat("clients stop %d",m_bStopped.load());
	}
}
