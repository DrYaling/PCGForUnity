#ifndef _TRANSFORM_MAP_H
#define _TRANSFORM_MAP_H
#include <stdlib.h>
#include <functional>
#include <map>
#include <vector>
#include "Logger/Logger.h"
struct testObj {
	int key;
	int value;
	float fvalue;
};
namespace transformMap
{
	/*typedef std::function<void(int)> std_int_setter;
	typedef std::function<void(testObj)> std_obj_setter;
	std::map<int, std::vector<std_int_setter>> map_01;
	std::map<int, std::vector<std_obj_setter>> map_02;*/
	void Initilize(std::string dir);
	std::string GetEngineDir();
	/*void SetValue(int value)
	{
		//LogFormat("SetValue %d", value);
	}
	void SetObjectValue(testObj obj)
	{
		//LogFormat("SetObjectValue %d", obj.key,obj.value,obj.fvalue);
	}
	class testClassObject {
	public:
		testClassObject() {}
		void Set(int value) {
			//LogFormat("testClassObject set %d", value);
		}
		void Init() {

		}
	};*/
	//testClassObject* test = new testClassObject();
	void ClearTransformMapTrees();
	/*{
		//map_01.clear();
		//map_02.clear();
	}*/
	void InitTransformMap(const char* engineDir);
	/*{
		Initilize(engineDir);
		/ *map_01.clear();
		map_02.clear();
		std::vector<std_int_setter> vSetter;
		vSetter.clear();
		std_int_setter func = std::bind(&testClassObject::Set, test, std::placeholders::_1);
		vSetter.push_back(func);
		func = std::bind(&SetValue, std::placeholders::_1);
		vSetter.push_back(func);
		map_01.insert(std::pair<int, std::vector<std_int_setter>>(0, vSetter));

		std::vector<std_obj_setter> oSetter;
		oSetter.clear();
		std_obj_setter funco = std::bind(SetObjectValue, std::placeholders::_1);
		oSetter.push_back(funco);
		map_02.insert(std::pair<int, std::vector<std_obj_setter>>(0, oSetter));* /
	}*/
	void TransformMapHandleSetter(int key, int value);
	/*{
		auto val = map_01.find(key);
		if (val != map_01.end())
		{
			for (auto func : map_01[key])
			{
				func(value);
			}
		}
	}*/
	void TransformMapHandleSetter(int key, testObj value);
	/*{
		auto val = map_02.find(key);
		if (val != map_02.end())
		{
			for (auto func : map_02[key])
			{
				func(value);
			}
		}
	}*/
}
#endif
