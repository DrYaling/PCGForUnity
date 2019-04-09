#include "MeshGenerator.h"
#include "Logger/Logger.h"

/*TerrianGenerator* pGenerator = nullptr;
std::vector<Vector3> vertexs[MAX_MESH_COUNT];
std::vector<Vector3> normals[MAX_MESH_COUNT];
std::vector<int32_t> indexes[MAX_MESH_COUNT];
int32_t _seed;
std::vector<int32_t> _args;
bool _optimalize;
bool _initilized = false;
void InitGenerator(int32_t seed, const int32_t* args, int32_t argSize, bool optimalize)
{
	if (nullptr == args)
	{
		_initilized = false;
		return;
	}
	_seed = seed;
	_initilized = true;
	_optimalize = optimalize;
	for (int i = 0; i < MAX_MESH_COUNT; i++)
	{
		vertexs[i].clear();
		indexes[i].clear();
		normals[i].clear();
	}
	_args.clear();
	for (int i = 0; i < argSize; i++)
	{
		_args.push_back(args[i]);
	}
}

void GenMeshData(int32_t type, int32_t* vSize)
{
	if (!_initilized)
	{
		return;
	}
	if (nullptr == vSize)
	{
		return;
	}
	switch (type)
	{
	case 0:
	{
		/ *auto mtg = new MountainGen(std::move(Vector3()), _arg0);
		mtg->Init(_seed, _arg1, _arg2, _optimalize);
		if (mtg)
		{
			mtg->Start(indexes, vertexs);
			vSize = vertexs.size();
			idxSize = indexes.size();
			pGenerator = mtg;
		}
		else
		{
			pGenerator = nullptr;
			vSize = idxSize = 0;
		}* /
	}
	break;
	case 1:
	{
		if (_args.size() < 7)
		{
			return;
		}
	}
	break;;
	default:
		break;
	}
}

void GetGeneratorData(Vector3 * pV, int32_t * pI, int arg0)
{
	if (!_initilized)
	{
		return;
	}
	if (arg0 >= MAX_MESH_COUNT || arg0 < 0)
	{
		return;
	}
	if (pV)
	{
		for (auto v : vertexs[arg0])
		{
			*pV++ = v;
		}
	}
	if (pI)
	{
		for (auto i : indexes[arg0])
		{
			*pI++ = i;
		}
	}
}

void GetGeneratorTrianglesData(int32_t * pI, int arg0)
{
	if (!_initilized)
	{
		return;
	}
	if (arg0 >= MAX_MESH_COUNT || arg0 < 0)
	{
		return;
	}
	if (pI)
	{
		for (auto i : indexes[arg0])
		{
			*pI++ = i;
		}
	}
}

int32_t * GetGeneratorTrianglesData(int arg0)
{
	if (!_initilized)
	{
		return nullptr;
	}
	if (arg0 >= MAX_MESH_COUNT || arg0 < 0)
	{
		return nullptr;
	}
	return indexes[arg0].data();
}

int32_t GetGeneratorVerticesData(Vector3 * pV, Vector3* pN, int index)
{
	if (!_initilized)
	{
		return 0;
	}
	if (index >= MAX_MESH_COUNT || index < 0)
	{
		return 0;
	}
	LogFormat("vertexs %d size %d", index, vertexs[index].size());
	if (pV)
	{
		for (auto v : vertexs[index])
		{
			*pV++ = v;
		}
	}
	//LogFormat("normals %d size %d", index, normals[index].size());
	if (pN)
	{
		for (auto n : normals[index])
		{
			*pN++ = n;
		}
	}
	return indexes[index].size();
}
*/
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
void Internal_InitTerrianMesh(int32_t instanceId, int32_t* args, int32_t argsize, ResizeIndicesCallBack cb)
{
	LogFormat("Internal_InitTerrianMesh %d", instanceId);
	TerrianMesh* mesh = Internal_GetTerrianMesh(instanceId);
	if (mesh)
	{
		LogFormat("Init Terrian %d", instanceId);
		mesh->Init(args, argsize, cb);
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
		mesh->RecaculateTriangles(lod);
	}
}

void Internal_RegisterTerrianMeshBinding(int32_t instance)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr == mTerrianBindings.end())
	{
		LogFormat("register %d", instance);
		auto terrian = new TerrianMesh();
		mTerrianBindings.insert(std::make_pair(instance, terrian));
	}
	else
	{
		LogErrorFormat("terrian of instance %d already exist!", instance);
	}
}

void Internal_ReleaseGenerator(int32_t instance)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->Release();
		delete mTerrianBindings[instance];
		mTerrianBindings.erase(itr);
	}
}
void Internal_InitMeshVerticeData(int32_t instance, G3D::Vector3 * vertices, int32_t size, int32_t mesh)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->InitTerrianVerticesData(vertices, size, mesh);
	}
}
void Internal_InitMeshNormalData(int32_t instance, G3D::Vector3 * normals, int32_t size, int32_t mesh)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->InitMeshNormalData(normals, size, mesh);
	}
}
void Internal_InitMeshUVData(int32_t instance, G3D::Vector2 * uvs, int32_t size, int32_t mesh, int32_t uv)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->InitMeshUVData(uvs, size, mesh, uv);
	}
}
void Internal_InitMeshTrianglesData(int32_t instance, int32_t * triangles, int32_t size, int32_t mesh, int32_t lod)
{
	auto itr = mTerrianBindings.find(instance);
	if (itr != mTerrianBindings.end())
	{
		mTerrianBindings[instance]->InitMeshTriangleData(triangles, size, mesh, lod);
	}
}
void Internal_InitMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection)
{
	auto itr = mTerrianBindings.find(instanceId);
	if (itr != mTerrianBindings.end())
	{
	}
}
NS_GNRT_END