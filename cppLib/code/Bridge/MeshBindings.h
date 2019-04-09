#ifndef _MESH_BINDINGS_H
#define _MESH_BINDINGS_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
#include "Generators/MeshGenerator.h"
typedef void(__stdcall * ResizeIndicesCallBack)(int32_t type, int32_t mesh, int32_t lod, int32_t size);
EXTERN_C_BEGIN


EXPORT_API void	 STD_CALL ReleaseMeshGenerator(int32_t instanceId)
{
	generator::Internal_ReleaseGenerator(instanceId);
}
EXPORT_API void RegisterTerrianMeshBindings(int32_t instanceId)
{
	generator::Internal_RegisterTerrianMeshBinding(instanceId);
}

EXPORT_API void	 STD_CALL InitTerrianMesh(int32_t instanceId,/* TerrianDataBinding& data,*/ int32_t* args, int32_t argsize, ResizeIndicesCallBack cb)
{
	LogFormat("InitTerrianMesh %d", instanceId);
	//generator::Internal_InitTerrianMesh(instanceId, data, args, argsize, cb);
}
EXPORT_API void STD_CALL InitMeshVerticeData(int32_t instanceId, G3D::Vector3* p, int32_t size, int32_t mesh)
{
	generator::Internal_InitMeshVerticeData(instanceId, p, size, mesh);
}
EXPORT_API void STD_CALL InitMeshNormalData(int32_t instanceId, G3D::Vector3* p, int32_t size, int32_t mesh)
{
	generator::Internal_InitMeshNormalData(instanceId, p, size, mesh);
}
EXPORT_API void STD_CALL InitMeshUVData(int32_t instanceId, G3D::Vector2* p, int32_t size, int32_t mesh, int32_t uv)
{
	generator::Internal_InitMeshUVData(instanceId, p, size, mesh, uv);
}
EXPORT_API void STD_CALL InitMeshTriangleData(int32_t instanceId, int32_t* p, int32_t size, int32_t mesh, int32_t lod)
{
	generator::Internal_InitMeshTrianglesData(instanceId, p, size, mesh, lod);
}
EXPORT_API void STD_CALL InitMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection)
{
	generator::Internal_InitMeshNeighbor(instanceId, neighborId, neighborDirection);
}

EXPORT_API void STD_CALL StartGenerateOrLoad(int32_t instanceId)
{
	generator::Internal_StartGenerateOrLoad(instanceId);
}
EXPORT_API void STD_CALL ResetLod(int32_t instanceId, int32_t lod)
{
	generator::Internal_ResetLod(instanceId, lod);
}

EXTERN_C_END

#endif