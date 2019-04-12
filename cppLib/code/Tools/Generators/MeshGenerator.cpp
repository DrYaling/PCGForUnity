#include "MeshGenerator.h"
#include "Logger/Logger.h"
#include "Terrian/TerrianMesh.h"
using namespace generator;
static std::map<int32_t, TerrianMesh*> mTerrianBindings;

NS_GNRT_START

static inline TerrianMesh* Internal_GetTerrianMesh(int32_t instance)
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
void Internal_InitTerrianMesh(int32_t instanceId, int32_t* args, int32_t argsize, MeshInitilizerCallBack cb, GeneratorNotifier notifier)
{
	//LogFormat("Internal_InitTerrianMesh %d", instanceId);
	TerrianMesh* mesh = Internal_GetTerrianMesh(instanceId);
	if (mesh)
	{
		//LogFormat("Init Terrian %d", instanceId);
		mesh->Init(args, argsize, cb, notifier);
	}
}

void Internal_StartGenerateOrLoad(int32_t instanceId)
{
	TerrianMesh* mesh = Internal_GetTerrianMesh(instanceId);
	if (mesh)
	{
		mesh->Start();
	}
}

void Internal_ResetLod(int32_t instanceId, int32_t lod)
{
	TerrianMesh* mesh = Internal_GetTerrianMesh(instanceId);
	if (mesh)
	{
		mesh->SetLod(lod);
	}
}

void Internal_RegisterTerrianMeshBinding(int32_t instance)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr == mTerrianBindings.end())
	{
		LogFormat("register %d", instance);
		auto terrian = new TerrianMesh(instance);
		mTerrianBindings.insert(std::make_pair(instance, terrian));
	}
	else
	{
		LogErrorFormat("terrian of instance %d already exist!", instance);
	}
}

void Internal_ReleaseGenerator(int32_t instance)
{
	LogFormat("Internal_ReleaseGenerator %d", instance);
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
void Internal_GetMeshVerticeData(int32_t instanceId, G3D::Vector3* pV, G3D::Vector3* pN, int32_t size, int32_t mesh)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instanceId]->GetTerrianVerticesData(pV, pN, size, mesh);
	}
}
/*
void Internal_GetMeshNormalData(int32_t instance, G3D::Vector3 * normals, int32_t size, int32_t mesh)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->GetMeshNormalData(normals, size, mesh);
	}
}*/
void Internal_GetMeshUVData(int32_t instance, G3D::Vector2 * uvs, int32_t size, int32_t mesh, int32_t uv)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->GetMeshUVData(uvs, size, mesh, uv);
	}
}

void Internal_GetMeshTrianglesData(int32_t instance, int32_t * triangles, int32_t size, int32_t mesh, int32_t lod)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->InitMeshTriangleData(triangles, size, mesh, lod);
	}
}
void Internal_ReloadMeshNormalData(int32_t instanceId, G3D::Vector3 * p, int32_t size, int32_t mesh, int32_t meshEdgePosition)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instanceId]->RecaculateNormal(p, size, mesh, meshEdgePosition);
	}
}
void Internal_SetMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection, bool reloadNormalIfLoaded)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
		auto neighbor = mTerrianBindings.find(neighborId);
		if (neighbor != mTerrianBindings.end())
		{
			itr->second->InitNeighbor(neighborDirection, neighbor->second, reloadNormalIfLoaded);
		}
		else
		{
			LogErrorFormat("mesh %d neighbor %d does not exist", instanceId, neighborId);
		}
	}
}
void Internal_OnNeighborLodChanged(int32_t instanceId, int32_t neighborId)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
		auto neighbor = mTerrianBindings.find(neighborId);
		if (neighbor != mTerrianBindings.end())
		{
			itr->second->OnNeighborLodChanged(neighbor->second);
		}
		else
		{
			LogErrorFormat("mesh %d neighbor %d does not exist", instanceId, neighborId);
		}
	}
}
void Internal_ReleaseAllMeshed()
{
	LogFormat("Internal_ReleaseAllMeshed %d",mTerrianBindings.size());
	for (auto itr = mTerrianBindings.begin(); itr != mTerrianBindings.end();)
	{
		safe_delete(itr->second);
		itr = mTerrianBindings.erase(itr);
	}
}
NS_GNRT_END