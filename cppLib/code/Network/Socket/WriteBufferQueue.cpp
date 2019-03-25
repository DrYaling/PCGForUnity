#include "WriteBufferQueue.h"

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

bool WriteBufferQueue::Push(char * data, int32 dataSize, int client)
{
	SocketWriteBuffer* freeBuffer = nullptr;
	bool foundBuffer = false;
	bool needFlush = true;
	for (auto itr = _writeList.begin(); itr != _writeList.end(); itr++)
	{
		auto buffer = *itr;
		if (buffer->clientId == client)
		{
			if (!buffer->active)
			{
				buffer->active = true;
				buffer->buffer.Resize(dataSize);
				buffer->buffer.Reset();
			}
			else if (buffer->buffer.GetActiveSize() < dataSize)
			{
				buffer->buffer.Resize(buffer->buffer.GetBufferSize() + dataSize - buffer->buffer.GetActiveSize());
			}
			buffer->buffer.Write(data, dataSize);
			foundBuffer = true;
			break;;
		}
		//found one free buffer
		else if (nullptr == freeBuffer && !buffer->active)
		{
			freeBuffer = buffer;
		}
	}
	if (!foundBuffer)
	{
		//use old one
		if (nullptr != freeBuffer)
		{
			freeBuffer->active = true;
			freeBuffer->buffer.Resize(dataSize);
			freeBuffer->buffer.Reset();
			freeBuffer->buffer.Write(data, dataSize);
		}
		//create a new buffer
		else if (_writeList.size() < MAX_WRITE_BUFFER_SIZE)
		{
			SocketWriteBuffer* buffer = new SocketWriteBuffer(client, data, dataSize);
			_writeList.push_back(buffer);
			needFlush = false;
		}
		//else ignore the buffer
		else
		{
			return false;
		}
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
	bool flushFlag = false;
	for (auto itr = _writeList.begin();itr != _writeList.end();)
	{
		if (!(*itr)->active)
		{
			for (auto itr1 = _writeList.begin(); itr1 != _writeList.end();)
			{
				if ((*itr1)->active)
				{
					//TODO NOT TESTED
					itr = _writeList.insert(itr, *itr1);
					_writeList.erase(itr1);
					flushFlag = true;
					break;
				}
			}
			//flushFlag 为 false说明后面全是inactive
			if (!flushFlag)
			{
				break;
			}
		}
		itr++;
	}
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

