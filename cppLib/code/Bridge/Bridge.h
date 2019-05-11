#ifndef _BEIDGE_H
#define _BEIDGE_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
//#include "Generators/MeshGenerator.h"
#include "MeshBindings.h"
#include <stdint.h>
#include "Threading/ThreadManager.h"
typedef void(__stdcall *CPPUpdateCallback)(int tick);
EXTERN_C_BEGIN
EXPORT_API void STD_CALL InitCppEngine(const char* engineDir)
{
	transformMap::InitTransformMap(engineDir);
}

EXPORT_API void STD_CALL RegisterLog(logger::CPPLogCallback callback)
{
	logger::SetLogCallBack(0, callback);
}
EXPORT_API void STD_CALL RegisterLogWarning(logger::CPPLogCallback callback)
{
	logger::SetLogCallBack(1, callback);
}

EXPORT_API void STD_CALL RegisterLogError(logger::CPPLogCallback callback)
{
	logger::SetLogCallBack(2, callback);
}
EXPORT_API void STD_CALL Update(int time_diff)
{
}
EXPORT_API void STD_CALL DestroyCPP()
{
	LogFormat("DestroyCPP");
	WorldMapBindings_Destroy();
	threading::ThreadManager::Destroy();
	transformMap::ClearTransformMapTrees();
	logger::ClearLogger();
}

EXTERN_C_END

#endif // _BEIDGE_H
