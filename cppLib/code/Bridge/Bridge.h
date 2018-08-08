#ifndef _BEIDGE_H
#define _BEIDGE_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
#include "Structure/Object.h"
typedef void(__stdcall *CPPUpdateCallback)(int tick);
#define STD_CALL __stdcall
EXTERN_C_BEGIN


EXPORT_API void STD_CALL InitCppEngine()
{
	transformMap::InitTransformMap();
}

EXPORT_API void STD_CALL HandleSetInt(int key, int value)
{
	transformMap::TransformMapHandleSetter(key, value);
}
EXPORT_API void STD_CALL HandleSetObject(int key, testObj value)
{
	transformMap::TransformMapHandleSetter(key, value);
}

EXPORT_API void STD_CALL RegisterLog(CPPLogCallback callback)
{
	logCallBack = callback;
}
EXPORT_API void STD_CALL RegisterLogWarning(CPPLogWarningCallback callback)
{
	logWarningCallBack = callback;
}

EXPORT_API void STD_CALL RegisterLogError(CPPLogErrorCallback callback)
{
	logErrorCallBack = callback;
}
EXPORT_API void STD_CALL Update(int time_diff)
{
	/*long ret = 0;
	for (int i = 0; i < 10000000; i++)
	{
		ret += i;
	}
	LogFormat("ret is %d", ret);*/
}
EXPORT_API void STD_CALL DestroyCPP()
{
	LogFormat("DestroyCPP"); 
	transformMap::ClearTransformMapTrees();
	ClearLogger();
}

EXTERN_C_END

#endif // _BEIDGE_H
