#ifndef _MESH_BINDINGS_H
#define _MESH_BINDINGS_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
#include "Generators/TerrainGenerator.h"
#include "generator.h"
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

EXPORT_API void	 STD_CALL InitTerrianMesh(int32_t instanceId, int32_t* args, int32_t argsize, float* heightMap, int32_t heightMapSize, MeshInitilizerCallBack cb)
{
	//LogFormat("InitTerrianMesh %d", instanceId);
	generator::Internal_InitTerrianMesh(instanceId, args, argsize,heightMap,heightMapSize, cb);
}
EXPORT_API void STD_CALL GetTerraniHeightMap(int32_t instanceId, float* heightMap, int32_t size1, int32_t size2)
{
	generator::Internal_GetTerraniHeightMap(instanceId, heightMap, size1, size2);
}
EXPORT_API void STD_CALL SetMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection)
{
	generator::Internal_SetMeshNeighbor(instanceId, neighborId, neighborDirection);
}

EXPORT_API void STD_CALL StartGenerateOrLoad(int32_t instanceId)
{
	generator::Internal_StartGenerateOrLoad(instanceId);
}

EXTERN_C_END

#endif