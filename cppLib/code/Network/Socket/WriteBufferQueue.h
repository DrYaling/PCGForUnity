#ifndef WRITE_BUFFER_QUEUE_H
#define WRITE_BUFFER_QUEUE_H
#endif
#include "MessageBuffer.h"
#include "define.h"
#define  MAX_WRITE_BUFFER_SIZE 128
struct SocketWriteBuffer
{
public:
	MessageBuffer buffer;
	int32 clientId;
	bool active;

	explicit SocketWriteBuffer(int32 id, char* buff, int32 buffSize) :
		clientId(id),
		active(true),
		buffer(buffSize)
	{
		buffer.Write(buff, buffSize);
	}
};
class WriteBufferQueue
{
public:
	WriteBufferQueue();
	~WriteBufferQueue();
	bool Push(char* data, int32 dataSize, int client);
	SocketWriteBuffer* Front();
	bool Pop();
	void Flush();
	inline int16 GetActiveSize() { return _activepos; }
private:
	std::vector<SocketWriteBuffer*> _writeList;
	int16 _activepos;

};
