#ifndef _BEIDGE_H
#define _BEIDGE_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
//#include "Generators/MeshGenerator.h"
#include "MeshBindings.h"
#include <stdint.h>
typedef void(__stdcall *CPPUpdateCallback)(int tick);
EXTERN_C_BEGIN

EXPORT_API void STD_CALL InitCppEngine(const char* engineDir)
{
	transformMap::InitTransformMap(engineDir);
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
	generator::Internal_ReleaseAllMeshed();
	transformMap::ClearTransformMapTrees();
	ClearLogger();
}
/*

EXPORT_API void	STD_CALL InitMeshGenerator(int32_t seed, const int32_t* args, int32_t argSize, bool optimalize)
{
	generator::InitGenerator(seed, args, argSize, optimalize);
}
EXPORT_API void STD_CALL GenerateMesh(int32_t type, int32_t* vSize)
{
	generator::GenMeshData(type, vSize);
}
/ *
EXPORT_API void STD_CALL GetMeshData(Vector3 * pV, int32_t * pI,int arg0)
{
	generator::GetGeneratorData(pV, pI, arg0);
}* /

EXPORT_API int32_t STD_CALL GetMeshVerticesData(Vector3 * pV,Vector3* pN, int arg0)
{
	return generator::GetGeneratorVerticesData(pV,pN, arg0);
}
EXPORT_API void STD_CALL GetMeshTrianglesData(int32_t * pI, int arg0)
{
	generator::GetGeneratorTrianglesData(pI, arg0);
}*/
static int internalCall()
{
	return -1;
}

EXTERN_C_END

#endif // _BEIDGE_H
