#include "safe_array.h"

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
float float_array::_ssize = 0;
float_array float_array::thread_unsafe_static_array_zero(0);
