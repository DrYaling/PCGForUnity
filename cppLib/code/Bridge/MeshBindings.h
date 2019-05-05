#ifndef _MESH_BINDINGS_H
#define _MESH_BINDINGS_H
#include "define.h"
#include "Logger/Logger.h"
#include "G3D/Vector3.h"
#include "Map/TransformMap.h"
#include "generator.h"
#include "MapGenerator/MapGenerator.h"
EXTERN_C_BEGIN
using namespace generator;
//map generator begin
EXPORT_API void	 STD_CALL WorldMapBindings_InitilizeWorldMap(MapGeneratorData data)
{
	sMapGenerator->Init(data);
	sMapGenerator->SetNativeDirectory(transformMap::GetEngineDir() + "/Map");
	sMapGenerator->StartRun();
}
EXPORT_API void	 STD_CALL WorldMapBindings_StopGeneration()
{
	sMapGenerator->Stop();
}
EXPORT_API void STD_CALL WorldMapBindings_WorkThreadRunner()
{
	sMapGenerator->WorkThreadEntry();
}
EXPORT_API void STD_CALL WorldMapBindings_SetGenerateCallBack(TerrainGenerationCallBack cb)
{
	sMapGenerator->SetCallBack(cb);
}
EXPORT_API uint32_t	 STD_CALL WorldMapBindings_GetNeighbor(uint32_t who, int32_t dir)
{
	auto ptr = sMapGenerator->GetTerrain(who);
	if (ptr)
	{
		return ptr->GetNeighbor((NeighborType)dir);
	}
	else
	{
		return 0;
	}
}
EXPORT_API void	 STD_CALL WorldMapBindings_InitTerrain(uint32_t who, float* heightMap, int32_t heightMapSize, float* splatMap, int32_t splatWidth, int32_t splatCount)
{
	sMapGenerator->InitTerrainInMainThread(who, heightMap, heightMapSize, splatMap, splatWidth, splatCount);
}

EXPORT_API void	 STD_CALL WorldMapBindings_UpdateInMainThread(int32_t diff)
{
	sMapGenerator->UpdateInMainThread(diff);
}

EXPORT_API void	 STD_CALL WorldMapBindings_Destroy()
{
	MapGenerator::Destroy();
}


#if false
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
	generator::Internal_InitTerrianMesh(instanceId, args, argsize, heightMap, heightMapSize, cb);
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
EXPORT_API void STD_CALL InitTerrainPainter(int32_t instanceId, float * alphaMap, int32_t sizeXy, int32_t splatCount)
{
	//LogFormat("InitTerrainPainter addr %d,size %d,count %d,f %f",alphaMap,sizeXy,splatCount,alphaMap[GetSplatMapIndex(0,1,0,sizeXy,splatCount)]);
	generator::Internal_InitTerrainPainter(instanceId, alphaMap, sizeXy, splatCount);
}
#endif

EXTERN_C_END

#endif