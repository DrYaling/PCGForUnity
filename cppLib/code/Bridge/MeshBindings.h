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
	if (MapGenerator::HasInstance())
	{
		sMapGenerator->Stop();
	}
}
EXPORT_API void STD_CALL WorldMapBindings_WorkThreadRunner()
{
	if (MapGenerator::HasInstance())
	{
		sMapGenerator->WorkThreadEntry();
	}
}
EXPORT_API void STD_CALL WorldMapBindings_SetGenerateCallBack(TerrainGenerationCallBack cb)
{
	if (MapGenerator::HasInstance())
	{
		sMapGenerator->SetCallBack(cb);
	}
}
EXPORT_API uint32_t	 STD_CALL WorldMapBindings_GetNeighbor(uint32_t who, int32_t dir)
{
	if (MapGenerator::HasInstance())
	{
		auto ptr = sMapGenerator->GetTerrain(who);
		uint32_t ret = 0;
		if (ptr)
		{
			ret = ptr->GetNeighbor((NeighborType)dir);
		}
		return ret;
	}
	return 0;
}
EXPORT_API void	 STD_CALL WorldMapBindings_InitTerrain(uint32_t who, float* heightMap, int32_t heightMapSize, float* splatMap, int32_t splatWidth, int32_t splatCount)
{
	if (MapGenerator::HasInstance())
	{
		sMapGenerator->InitTerrainInMainThread(who, heightMap, heightMapSize, splatMap, splatWidth, splatCount);
	}
}

EXPORT_API void	 STD_CALL WorldMapBindings_UpdateInMainThread(int32_t diff)
{
	if (MapGenerator::HasInstance())
		sMapGenerator->UpdateInMainThread(diff);
}

EXPORT_API void	 STD_CALL WorldMapBindings_Destroy()
{
	MapGenerator::Destroy();
}

EXTERN_C_END

#endif