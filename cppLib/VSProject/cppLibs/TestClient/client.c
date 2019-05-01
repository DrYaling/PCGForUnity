#include "Network/Socket/CSocket.h"
//#include "Logger/Logger.h"
#include "Network/Proto/punch.pb.h"
#include <iostream>
#include <thread>
#include "Logger/Logger.h"
#include <stdio.h>
#include "Network/Socket/KcpClient.h"
#include "Network/Socket/SocketTime.h"
int main()
{
	//sleep(100);
	int client_count = 50;
	while (client_count--)
	{
		std::thread* thread = new std::thread([]()->void
		{
			KcpClient* client = new KcpClient(0);
			SocketTime.timeStampSinceStartUp = 0;
			client->SetAddress("127.0.0.1", 8081);
			auto callback = [&client](int cmd, const uint8* buff, int size)->bool {
				LogFormat("recv server msg cmd :%d,content:%s,msg size %d", cmd, buff, size);
				//socket.SetRecvCallback(nullptr);
				return true;
			};
			std::mutex mtx;
			std::thread t([&]()->void {
				while (true)
				{
					sleep(15);
					SocketTime.Update(15);
					{
						std::lock_guard<std::mutex> lck(mtx);
						//LogFormat("Socket time %d", SocketTime.timeStampSinceStartUp);
						client->Update(15);
					}
				}
			});
			t.detach();
			client->SetReceiveCallBack(callback);
			client->Connect();
			while (!client->IsConnected())
			{
				//socket.Send(NULL, 0);
				sleep(1000);
			}
			char buff[345] = { "9" };
			char data[2550] = { 5 };
			int i = 43;
			Log("connect to 127.0.0.1:8081 success");

			while (i-- > 0)
			{
				PacketHeader header;
				header.Size = sizeof(buff);
				header.Command = 7;
				memcpy(data, &header, sizeof(header));
				sprintf_s(data + sizeof(header), header.Size, "%s", buff);
				sleep(3276);
				{
					std::lock_guard<std::mutex> lck(mtx);
					client->Send(data, sizeof(header) + sizeof(buff), false);
				}
				/*client.Send(data, sizeof(header) + sizeof(buff) + 1, true);
				LogFormat("send buff size %d", sizeof(header) + sizeof(buff) + 1);
				client.Send(data, sizeof(header) + sizeof(buff) + 2, true);
				LogFormat("send buff size %d", sizeof(header) + sizeof(buff) + 2);
				client.Send(data, sizeof(header) + sizeof(buff) + 3, true);
				LogFormat("send buff size %d", sizeof(header) + sizeof(buff) + 3);*/
			}
		});
	}
	
	//client.Close();
	//SocketTime_t::Destroy();
	while (true)
	{
		sleep(1);
	}
	return 0;
}