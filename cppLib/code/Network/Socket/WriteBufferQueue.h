#ifndef WRITE_BUFFER_QUEUE_H
#define WRITE_BUFFER_QUEUE_H
#endif
#include "MessageBuffer.h"
#include "define.h"
#include "SocketConfig.h"
#define  MAX_WRITE_BUFFER_SIZE 128
struct SocketWriteBuffer
{
public:
	MessageBuffer buffer;
	SockAddr_t addr;
	bool active;

	explicit SocketWriteBuffer(const SockAddr_t& adr,const char* buff, int32 buffSize) :
		addr(adr),
		active(true),
		buffer(buffSize)
	{
		buffer.Write(buff, buffSize);
	}
};
const int size = sizeof(SocketWriteBuffer);
class WriteBufferQueue
{
public:
	WriteBufferQueue();
	~WriteBufferQueue();
	bool Push(const char* data, int32 dataSize, const SockAddr_t& adr);
	SocketWriteBuffer* Front();
	bool Pop();
	void Flush();
	inline int16 GetActiveSize() { return _activepos; }
private:
	std::vector<SocketWriteBuffer*> _writeList;
	int16 _activepos;

};
