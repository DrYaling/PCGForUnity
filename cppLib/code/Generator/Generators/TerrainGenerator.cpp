#if  false

#include "TerrainGenerator.h"
#include "Logger/Logger.h"
#include "Terrain/Terrain.h"
#include "MapGenerator/MapGenerator.h"
using namespace generator;
static std::map<int32_t, Terrain*> mTerrianBindings;

NS_GNRT_START

static inline Terrain* Internal_GetTerrianMesh(int32_t instance)
{

	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		return mTerrianBindings[instance];
	}
	else
	{
		return nullptr;
	}
}
void Internal_InitTerrianMesh(int32_t instanceId, int32_t* args, int32_t argsize, float* heightMap, int32_t heightMapSize, MeshInitilizerCallBack cb)
{
	//LogFormat("Internal_InitTerrianMesh %d", instanceId);
	Terrain* mesh = Internal_GetTerrianMesh(instanceId);
	if (mesh)
	{
		mesh->Init(args, argsize, heightMap, heightMapSize, cb);
	}
}

void Internal_StartGenerateOrLoad(int32_t instanceId)
{
	Terrain* mesh = Internal_GetTerrianMesh(instanceId);
	if (mesh)
	{
		mesh->Start();
	}
}

void Internal_RegisterTerrianMeshBinding(int32_t instance)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr == mTerrianBindings.end())
	{
		//LogFormat("register %d", instance);
		auto terrian = new Terrain(instance);
		mTerrianBindings.insert(std::make_pair(instance, terrian));
	}
	else
	{
		LogErrorFormat("terrian of instance %d already exist!", instance);
	}
}

void Internal_ReleaseGenerator(int32_t instance)
{
	//LogFormat("Internal_ReleaseGenerator %d", instance);
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		delete mTerrianBindings[instance];
		mTerrianBindings.erase(itr);
	}
}
void Internal_FlushMeshGenerator(int32_t instance)
{
	//LogFormat("Internal_FlushMeshGenerator %d", instance);
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		itr->second->Release();
	}
}
void Internal_GetTerraniHeightMap(int32_t instanceId, float * heightMap, int32_t size1, int32_t size2)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instanceId]->GetHeightMap(heightMap, size1, size2);
	}
}
void Internal_SetMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
		auto neighbor = mTerrianBindings.find(neighborId);
		if (neighbor != mTerrianBindings.end())
		{
			itr->second->InitNeighbor((NeighborType)neighborDirection, neighbor->second);
		}
		else
		{
			LogErrorFormat("mesh %d neighbor %d does not exist", instanceId, neighborId);
		}
	}
}
void Internal_InitTerrainPainter(int32_t instanceId, float * alphaMap, int32_t sizeXy, int32_t splatCount)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instanceId]->AutoGenSplatMap(alphaMap, sizeXy, splatCount);
	}
}
void Internal_ReleaseAllMeshed()
{
	//LogFormat("Internal_ReleaseAllMeshed %d",mTerrianBindings.size());
	for (auto itr = mTerrianBindings.begin(); itr != mTerrianBindings.end();)
	{
		safe_delete(itr->second);
		itr = mTerrianBindings.erase(itr);
	}
}
NS_GNRT_END
#endif