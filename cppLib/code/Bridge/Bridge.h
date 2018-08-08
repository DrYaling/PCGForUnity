#ifndef _BEIDGE_H
#define _BEIDGE_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
#include <stdint.h>
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
EXPORT_API void SetIntTest(int32_t input)
{

}
class EXPORT_COREMODULE vector3 {
public:
	int32_t x, y, z;
};
vector3 v;
EXPORT_API void SetObjtest(vector3& v)
{
	LogFormat("x:%d,y:%d,z:%d",v.x, v.y, v.z);
}
EXPORT_API int32_t GetIntTest()
{
	return 111;
}
EXPORT_API vector3& GetObjTest()
{
	return v;
}
EXPORT_API void STD_CALL DestroyCPP()
{
	LogFormat("DestroyCPP"); 
	transformMap::ClearTransformMapTrees();
	ClearLogger();
}

static int internalCall()
{
	return -1;
}

EXTERN_C_END

#endif // _BEIDGE_H
