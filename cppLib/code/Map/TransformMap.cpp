#include "TransformMap.h"
namespace transformMap
{
	std::string pEngineDir;
	void Initilize(std::string dir)
	{
		pEngineDir = dir;
	}

	std::string GetEngineDir()
	{
		return pEngineDir;
	}

	void ClearTransformMapTrees()
	{
	}

	void InitTransformMap(const char * engineDir)
	{
		Initilize(engineDir);
	}

	void TransformMapHandleSetter(int key, int value)
	{
	}

	void TransformMapHandleSetter(int key, testObj value)
	{
	}
}