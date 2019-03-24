#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include <map>
#include <queue>
#include "CSocket.h"
#include "Network\Socket\MessageBuffer.h"

typedef std::function<bool(int, uint8*,int)> SocketDataReceiveHandler;
class SocketServer {
	struct SocketWriteBuffer
	{
	public:
		MessageBuffer buffer;
		int32 clientId;

		explicit SocketWriteBuffer(int32 id, char* buff,int32 buffSize) :
			clientId(id),
			buffer(buffSize)
		{
			buffer.Write(buff, buffSize);
		}
	};
public:
	SocketServer(SocketType sock);
	~SocketServer();
	bool StartUp();
	bool Stop();
	bool SetAddress(const char * ip,unsigned short port);
	int GetClientCount() {
		return m_mSocketClients.size();
	}
	void Update();
	bool Send(char* data, int32 dataSize, int client);
protected:
	void ReadHandlerInternal(int size, const char* buffer);
	void ReadHandler();
	ReadDataHandlerResult ReadDataHandler();
	bool ReadHeaderHandler();
	void SendDataHandler(int size);
	int GetNewClientId() { return ++m_nClientIndex; }
private:
	SocketServer() {}
	MessageBuffer& GetReadBuffer() { return m_readBuffer; }
	void WriteThread();
	bool CloseClient(const SockAddr_t& client);
	bool ContainsRemote(const SockAddr_t& remote);
private:
	Socket * m_pSocket;
	std::queue<SocketWriteBuffer> _writeQueue;
	MessageBuffer m_headerBuffer;
	MessageBuffer m_packetBuffer;
	MessageBuffer m_readBuffer;
	std::mutex _sendLock;
	std::map<SockAddr_t, int16> m_mSocketClients;
	SocketDataReceiveHandler m_pDataHandler;
	int m_nClientIndex;
};
#endif // !SOCKET_SERVER_H
