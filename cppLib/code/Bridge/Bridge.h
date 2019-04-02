#ifndef _BEIDGE_H
#define _BEIDGE_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
#include "Generators/MeshGenerator.h"
#include <stdint.h>
typedef void(__stdcall *CPPUpdateCallback)(int tick);
#define STD_CALL __stdcall
EXTERN_C_BEGIN

EXPORT_API void STD_CALL InitCppEngine()
{
	LogFormat("InitCppEngine");
	transformMap::InitTransformMap();
}

EXPORT_API void STD_CALL HandleSetInt(int key, int value)
{
	LogFormat("HandleSetInt");
	transformMap::TransformMapHandleSetter(key, value);
}
EXPORT_API void STD_CALL HandleSetObject(int key, testObj value)
{
	LogFormat("HandleSetObject");
	transformMap::TransformMapHandleSetter(key, value);
}

EXPORT_API void STD_CALL RegisterLog(CPPLogCallback callback)
{
	SetLogCallBack(0, callback);
}
EXPORT_API void STD_CALL RegisterLogWarning(CPPLogCallback callback)
{
	SetLogCallBack(1, callback);
}

EXPORT_API void STD_CALL RegisterLogError(CPPLogCallback callback)
{
	SetLogCallBack(2, callback);
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
EXPORT_API void STD_CALL SetIntTest(int32_t input)
{

}
EXPORT_API struct vector3 {
	float x, y, z;
};
vector3 v;
EXPORT_API void  STD_CALL SetObjtest(vector3& v)
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

EXPORT_API void	STD_CALL InitMeshGenerator(int32_t seed, int32_t depth,int32_t step,int32_t maxHeight, Vector3 pos,bool usePerlin)
{
	LogFormat("InitMeshGenerator");
	generator::InitGenerator(pos,seed, depth,step, maxHeight, usePerlin);
}
EXPORT_API void	 STD_CALL ReleaseMeshGenerator()
{
	generator::ReleaseGenerator();
}
EXPORT_API void STD_CALL GenerateMesh(int32_t type, int32_t& vSize,int32_t& idxSize)
{
	switch (type)
	{
	case 0:
		generator::GenrateMountain(vSize,idxSize);
	default:
		break;
	}
}
EXPORT_API void STD_CALL GetMeshData(Vector3 * pV, int32_t * pI)
{
	generator::GetGeneratorData(pV, pI);
}

static int internalCall()
{
	return -1;
}

EXTERN_C_END

#endif // _BEIDGE_H
