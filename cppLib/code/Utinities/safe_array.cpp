#include "safe_array.h"

float_array::float_array(uint32_t size):
	_size(nullptr),
	ptr(nullptr), externArray(false)
{
	_copy = false;
	_ssize = 0;
	if (size)
	{
		ptr = new float[size + 1];
		if (ptr)
		{
			memset(ptr, 0, sizeof(float) * (size + 1));
			ptr[size] = (float&)size;
			_size = &ptr[size];
		}
	}
}

float_array::float_array(float* externPtr, uint32_t size):
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
			memset(ptr, 0, sizeof(float) * (size));
			_size = new float;
			*_size = (float&)size;
		}
	}
}


void float_array::Init(uint32_t size)
{
	LogFormat("float_array::Init size %d", size);
	if (_size && size == (uint32_t&)*_size && !_copy && !externArray)
	{
		LogErrorFormat("float_array::Init size0 %d", size);
		return;
	}
	else if (ptr && !_copy && !externArray)
	{
		safe_delete_array(ptr);
	}
	else if (!_copy && externArray)
		safe_delete(_size);
	_copy = false;
	externArray = false;
	ptr = nullptr;
	if (size)
	{
		ptr = new float[size + 1];
	}
	if (ptr)
	{
		memset(ptr, 0, sizeof(float)*(size + 1));
		ptr[size] = (float&)size;
		_size = &ptr[size];
	}
	else
		_size = nullptr;
	LogFormat("float_array::Init size1 %d", size);
}

void float_array::Init(float * externPtr, uint32_t size)
{
	LogFormat("float_array::Init ext");
	if (_size && size == (uint32_t&)*_size && externPtr == ptr)
	{
		LogErrorFormat("float_array::Init ext 0");
		return;
	}
	if (externPtr == ptr)
	{
		LogErrorFormat("float_array::Init ext 1");
		return;
	}
	else if (ptr && !externPtr && !_copy)
	{
		safe_delete_array(ptr);
	}
	_copy = false;
	externArray = true;
	ptr = externPtr;
	if (ptr)
	{
		memset(ptr, 0, sizeof(float)*(size));
		_size = new float;
		*_size = (float&)size;
	}
	else
		_size = nullptr;
	LogFormat("float_array::Initext  2");
}

float_array::~float_array()
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

void float_array::copy(const float_array& from) const
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

/**
 * \brief 
 * \param from 
 * \param fsize 
 */
void float_array::copy(const float* from, uint32_t fsize) const
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

void float_array::copyTo(float* target, uint32_t copy_size)
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

void float_array::copyTo(float* target) const
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

float float_array::_ssize = 0;
float_array float_array::thread_unsafe_static_array_zero(0);
