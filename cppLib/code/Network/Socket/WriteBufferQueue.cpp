#include "WriteBufferQueue.h"

#include <algorithm> 
WriteBufferQueue::WriteBufferQueue():_activepos(0)
{
}

WriteBufferQueue::~WriteBufferQueue()
{
	for (auto itr = _writeList.begin();itr!= _writeList.end();)
	{
		delete *itr;
		itr = _writeList.erase(itr);
	}
}

bool WriteBufferQueue::Push(const char * data, int32 dataSize, const SockAddr_t& adr)
{
	SocketWriteBuffer* freeBuffer = nullptr;
	bool needFlush = true;
	for (auto itr = _writeList.begin(); itr != _writeList.end(); itr++)
	{
		auto buffer = *itr;
		if (nullptr == freeBuffer && !buffer->active)
		{
			freeBuffer = buffer;
			break;
		}
	}
	//use old one
	if (nullptr != freeBuffer)
	{
		freeBuffer->active = true;
		freeBuffer->addr = adr;
		freeBuffer->buffer.Resize(dataSize);
		freeBuffer->buffer.Reset();
		freeBuffer->buffer.Write(data, dataSize);
	}
	//create a new buffer
	else if (_writeList.size() < MAX_WRITE_BUFFER_SIZE)
	{
		SocketWriteBuffer* buffer = new SocketWriteBuffer(adr, data, dataSize);
		_writeList.push_back(buffer);
		_activepos = _writeList.size();
		needFlush = false;
	}
	//else ignore the buffer
	else
	{
		return false;
	}
	if (needFlush)
	{
		Flush();
	}
	return true;
}

SocketWriteBuffer * WriteBufferQueue::Front()
{
	return _writeList.at(0);
}

bool WriteBufferQueue::Pop()
{
	if (_writeList.size() > 0)
	{
		_writeList.at(0)->active = false;
		Flush();
		return true;
	}
	return false;
}
//heavy styl to flush
void WriteBufferQueue::Flush()
{
	std::sort(_writeList.begin(), _writeList.end(), [](SocketWriteBuffer* a, SocketWriteBuffer* b)->bool {
		return !a->active && b->active;
	});
	_activepos = 0;
	for (auto buffer:_writeList)
	{
		if (!buffer->active)
		{
			break;
		}
		_activepos++;
	}
}

