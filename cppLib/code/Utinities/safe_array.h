#ifndef safe_array_h
#define safe_array_h
#include "define.h"
#include <memory>
#include "Logger/Logger.h"
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
		return _size;
	}
	T& operator[](const uint32_t pos)
	{
		if (pos < _size)
			return ptr[pos];
		else
		{
			LogErrorFormat("get array out of size %d", _size);
			return _static;
		}
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
//extern ptr array is not cache friendly
class float_array
{
public:
	float_array() :
		_size(nullptr),
		ptr(nullptr),
		externArray(false)
	{
		_ssize = 0;
	}
	float_array(uint32_t size) :
		_size(nullptr),
		ptr(nullptr)
	{
		_copy = false;
		_ssize = 0;
		if (size)
		{
			ptr = new float[size + 1];
			if (ptr)
			{
				memset(ptr, 0, sizeof(float)*(size + 1));
				ptr[size] = (float&)size;
				_size = &ptr[size];
			}
		}
	}
	float_array(float* externPtr, uint32_t size) :
		_size(nullptr),
		ptr(nullptr)
	{
		_copy = false;
		_ssize = 0;
		if (size)
		{
			externArray = true;
			ptr = externPtr;
			if (ptr)
			{
				memset(ptr, 0, sizeof(float)*(size));
				_size = new float;
				*_size = (float&)size;
			}
		}
	}
	float_array(const float_array& _r) :
		_size(_r._size),
		externArray(false),
		ptr(_r.ptr)
	{
		_copy = true;
	}
	float_array(const float_array&&) = delete;
	void Init(uint32_t size);
	void Init(float* externPtr, uint32_t size);
	~float_array()
	{
		if (_copy)
			return;
		if (!externArray)
		{
			safe_delete_array(ptr);
		}
		else
		{
			safe_delete(_size);
		}
		_size = nullptr;
	}
	void copy(const float_array& from)
	{
		LogFormat("float array copy 00");
		if (!ptr || !from.ptr)
		{
			LogErrorFormat("float array is empty");
			return;
		}
		if (size() != from.size())
		{
			LogErrorFormat("float array size not equal!");
			return;
		}
		memcpy(ptr, from.ptr, size() * sizeof(float));
		LogFormat("float array copy 11");
	}
	void copy(const float* from, uint32_t fsize)
	{
		LogFormat("float array copy 0");
		if (!ptr || !from)
		{
			LogErrorFormat("float array is empty");
			return;
		}
		if (size() != fsize)
		{
			LogErrorFormat("float array size not equal!");
			return;
		}
		memcpy(ptr, from, fsize * sizeof(float));
		LogFormat("float array copy 1");
	}
	void copyTo(float* target, uint32_t copy_size)
	{
		LogFormat("float array copyTo 00");
		if (!ptr || !target)
		{
			LogErrorFormat("float array is empty");
			return;
		}
		if (size() < copy_size)
		{
			LogErrorFormat("float array size is smaller than given!");
			return;
		}
		memcpy(target, ptr, copy_size * sizeof(float));
		LogFormat("float array copyTo 22");
	}
	void copyTo(float* target)
	{
		if (!ptr || !target)
		{
			LogErrorFormat("float array is empty");
			return;
		}
		LogFormat("float array copyTo 0");
		memcpy(target, ptr, size() * sizeof(float));
		LogFormat("float array copyTo 1");
	}
	uint32_t size() const
	{
		LogFormat("size d %d", _size ? (uint32_t&)*_size : 0);
		return _size ? (uint32_t&)*_size : 0;
	}
	float& operator[](const uint32_t pos)
	{
		LogFormat("set/get array at %d,addr %d,ext %d,c %d", pos, ptr,externArray,_copy);
		if (pos < (uint32_t&)_size)
			return ptr[pos];
		else
		{
			LogErrorFormat("get array out of size %d", (uint32_t&)_size);
			float* pptr = nullptr;//tell c# stack trace
			pptr[0] = 1;
			throw std::exception("get array out of size");
		}
	}
	float_array& operator =(const float_array& r)
	{
		LogFormat("float_array = ex %d,c %d,size %d,addr %d,%d", externArray, _copy, size(), ptr, _size);
		if (!externArray && !_copy)
		{
			if (ptr)
			{
				LogFormat("float_array = 1");
				safe_delete_array(ptr);
			}
		}
		if (externArray && !_copy)
		{
			LogFormat("float_array = 2");
			safe_delete(_size);
		}
		LogFormat("float_array = 3");
		_size = r._size;
		externArray = false;
		LogFormat("float_array = 4");
		ptr = r.ptr;
		_copy = true;
		LogFormat("float_array = 5");
		return *this;
	}
	float* data()
	{
		return ptr;
	}
	const float* data()const
	{
		return ptr;
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
public:
	static float_array thread_unsafe_static_array_zero;
private:
	static float _ssize;
	float* _size;
	float* ptr;
	bool externArray;
	bool _copy;
};
//fast float safe array
//cache friendly float array
template<uint32_t _size>
class float_array_fixed
{
	friend class float_array;
public:
	float_array_fixed() :
		ptr(nullptr)
	{
		float_array::_ssize = 0;
		ptr = new float[_size];
		memset(ptr, 0, sizeof(float)*(_size));
	}
	~float_array_fixed()
	{
		if (ptr)
		{
			delete[] ptr;
			ptr = nullptr;
		}
	}
	inline uint32_t size()
	{
		//LogFormat("size f %f,size d %d", _size);
		return _size;
	}
	inline float& operator[](const uint32_t pos)
	{
		if (pos < _size)
			return ptr[pos];
		else
		{
			LogErrorFormat("get array out of size %d", _size);
			return float_array::_ssize;
		}
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
	float* data()
	{
		return ptr;
	}
	const float* data() const
	{
		return ptr;
	}
private:
	float* ptr;
};
#endif // !safe_array_h
