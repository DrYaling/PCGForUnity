#ifndef _BEIDGE_H
#define _BEIDGE_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
typedef void(__stdcall *CPPUpdateCallback)(int tick);
EXTERN_C_BEGIN


EXPORT_API void InitCppEngine()
{
	transformMap::InitTransformMap();
}

EXPORT_API void HandleSet(int key, int value)
{
	transformMap::TransformMapHandleSetter(key, value);
}

EXPORT_API void RegisterLog(CPPLogCallback callback)
{
	logCallBack = callback;
}
EXPORT_API void RegisterLogWarning(CPPLogWarningCallback callback)
{
	logWarningCallBack = callback;
}

EXPORT_API void RegisterLogError(CPPLogErrorCallback callback)
{
	logErrorCallBack = callback;
}
EXPORT_API void Update(int time_diff)
{
	long ret = 0;
	for (int i = 0; i < 10000000; i++)
	{
		ret += i;
	}
	LogFormat("ret is %d", ret);
}
EXPORT_API void DestroyCPP()
{
	LogFormat("DestroyCPP"); 
	logErrorCallBack = nullptr;
	logCallBack = nullptr;
	logWarningCallBack = nullptr;

}

EXTERN_C_END

#endif // _BEIDGE_H
