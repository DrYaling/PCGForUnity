#ifndef _TRANSFORM_MAP_H
#define _TRANSFORM_MAP_H
#include <stdlib.h>
#include <functional>
#include <map>
#include <vector>
#include "Logger/Logger.h"
namespace transformMap 
{
	typedef std::function<void(int)> std_setter;
	std::map<int, std::vector<std_setter>> map_01;

	void SetValue(int value)
	{
		LogFormat("SetValue %d", value);
	}
	class testClassObject {
	public:
		testClassObject() {}
		void Set(int value) {
			LogFormat("testClassObject set %d", value);
		}
		void Init() {

		}
	};
	testClassObject* test = new testClassObject();
	void InitTransformMap()
	{
		map_01.clear();
		std::vector<std_setter> vSetter;
		vSetter.clear();

		std_setter func = std::bind(&testClassObject::Set, test, std::placeholders::_1);
		vSetter.push_back(func);
		func = std::bind(&SetValue, std::placeholders::_1);
		vSetter.push_back(func);
		map_01.insert(std::pair<int, std::vector<std_setter>>(0, vSetter));
	}
	void TransformMapHandleSetter(int key, int value)
	{
		auto val = map_01.find(key);
		if (val != map_01.end())
		{
			for (auto func : map_01[key])
			{
				func(value);
			}
		}
	}
}
#endif
