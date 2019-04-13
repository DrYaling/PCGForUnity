#ifndef _MESH_BINDINGS_H
#define _MESH_BINDINGS_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
#include "Generators/MeshGenerator.h"
#include "Generators/generator.h"
EXTERN_C_BEGIN


EXPORT_API void	 STD_CALL ReleaseMeshGenerator(int32_t instanceId)
{
	generator::Internal_ReleaseGenerator(instanceId);
}
EXPORT_API void	 STD_CALL FlushMeshGenerator(int32_t instanceId)
{
	generator::Internal_FlushMeshGenerator(instanceId);
}
EXPORT_API void RegisterTerrianMeshBindings(int32_t instanceId)
{
	generator::Internal_RegisterTerrianMeshBinding(instanceId);
}

EXPORT_API void	 STD_CALL InitTerrianMesh(int32_t instanceId, int32_t* args, int32_t argsize, MeshInitilizerCallBack cb, GeneratorNotifier notifier)
{
	LogFormat("InitTerrianMesh %d", instanceId);
	generator::Internal_InitTerrianMesh(instanceId, args, argsize, cb, notifier);
}
EXPORT_API void STD_CALL GetTerraniHeightMap(int32_t instanceId, float* heightMap, int32_t size1, int32_t size2)
{
	generator::Internal_GetTerraniHeightMap(instanceId, heightMap, size1, size2);
}
EXPORT_API void STD_CALL GetMeshVerticeData(int32_t instanceId, G3D::Vector3* pV, G3D::Vector3* pN, int32_t size, int32_t mesh)
{
	generator::Internal_GetMeshVerticeData(instanceId, pV, pN, size, mesh);
}
EXPORT_API void STD_CALL GetMeshUVData(int32_t instanceId, G3D::Vector2* p, int32_t size, int32_t mesh, int32_t uv)
{
	generator::Internal_GetMeshUVData(instanceId, p, size, mesh, uv);
}

EXPORT_API void STD_CALL GetMeshTriangleData(int32_t instanceId, int32_t* p, int32_t size, int32_t mesh, int32_t lod)
{
	generator::Internal_GetMeshTrianglesData(instanceId, p, size, mesh, lod);
}
EXPORT_API void STD_CALL ReloadMeshNormalData(int32_t instanceId, G3D::Vector3 * p, int32_t size, int32_t mesh, int32_t meshEdgePosition)
{
	generator::Internal_ReloadMeshNormalData(instanceId, p, size, mesh, meshEdgePosition);
}
EXPORT_API void STD_CALL SetMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection)
{
	generator::Internal_SetMeshNeighbor(instanceId, neighborId, neighborDirection);
}
EXPORT_API void STD_CALL NeighborLodHasChanged(int32_t instanceId, int32_t neighborId)
{
	generator::Internal_OnNeighborLodChanged(instanceId, neighborId);
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