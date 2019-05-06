#ifndef safe_array_h
#define safe_array_h
#include "define.h"
#include <memory>
//#include "Logger/Logger.h"
template<class T>
class safe_array
{
public:
	safe_array() :
		_size(0),
		ptr(nullptr)
	{
	}
	safe_array(uint32_t size) :
		_size(0),
		ptr(nullptr)
	{
		ptr = new T[size];
		if (ptr)
		{
			_size = size;
		}
	}
	void Init(uint32_t size)
	{
		if (size == _size)
			return;
		else if (ptr)
		{
			delete[] ptr;
		}
		ptr = new T[size];
		if (ptr)
		{
			_size = size;
		}
	}
	~safe_array()
	{
		if (ptr)
		{
			delete[] ptr;
			ptr = nullptr;
		}
	}
	uint32_t size()
	{
		return _size ;
	}
	T& operator[](const uint32_t pos)
	{
		if (ptr && pos < _size)
			return ptr[pos];
		else
			return _static;
	}
	void Print()
	{
		/*if (_size)
		{
			LogFormat("array size %d, data addr %d,size addr %d", size(), ptr, &_size);
		}
		else
		{
			LogFormat("null");
		}*/
	}
private:
	T _static;
	uint32_t _size;
	T* ptr;
};
//fast float safe array
//cache friendly float array
class float_array
{
public:
	float_array() :
		_size(nullptr),
		ptr(nullptr)
	{
	}
	float_array(uint32_t size) :
		_size(nullptr),
		ptr(nullptr)
	{
		ptr = new float[size + 1];
		memset(ptr, 0, sizeof(float)*(size + 1));
		ptr[size] = (float&)size;
		if (ptr)
		{
			_size = &ptr[size];
		}
	}
	void Init(uint32_t size)
	{
		if (size == (uint32_t&)*_size)
			return;
		else if (ptr)
		{
			delete[] ptr;
		}
		ptr = new float[size + 1];
		memset(ptr, 0, sizeof(float)*(size + 1));
		ptr[size] = (float&)size;
		if (ptr)
		{
			_size = &ptr[size];
		}
		else
			_size = nullptr;
	}
	~float_array()
	{
		if (ptr)
		{
			delete[] ptr;
			ptr = nullptr;
		}
		_size = nullptr;
	}
	uint32_t size()
	{
		//LogFormat("size f %f,size d %d", _size);
		return _size ? (uint32_t&)*_size : 0;
	}
	float& operator[](const uint32_t pos)
	{
		if (ptr && pos < (uint32_t&)_size)
			return ptr[pos];
		else
			return _ssize;
	}
	void Print()
	{
		/*if (ptr)
		{
			LogFormat("farray size %d, data addr %d,size addr %d", size(), ptr, _size);
		}
		else
		{
			LogFormat("null");
		}*/
	}
private:
	static float _ssize;
	float* _size;
	float* ptr;
};
float float_array::_ssize = 0;
#endif // !safe_array_h
