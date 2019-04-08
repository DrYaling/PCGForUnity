#include "MeshGenerator.h"
#include "Logger/Logger.h"
#include "TerrianGenerator/Mountain.h"
#include "TerrianGenerator/Diamond_Square.h"
NS_GNRT_START

TerrianGenerator* pGenerator = nullptr;
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
		/*auto mtg = new MountainGen(std::move(Vector3()), _arg0);
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
		}*/
	}
	break;
	case 1:
	{
		if (_args.size() < 7)
		{
			return;
		}
		auto dsg = new Diamond_Square(_seed, _args[0], _args[1]);

		float h[4] = { _args[3],_args[4],_args[5],_args[6] };
		dsg->Start(h);
		dsg->GenerateTerrian(indexes, vertexs,normals, _args[2]);
		for (int i = 0; i < MAX_MESH_COUNT; i++)
			vSize[i] = vertexs[i].size();
		pGenerator = dsg;
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

void ReleaseGenerator()
{
	for (int i = 0; i < MAX_MESH_COUNT; i++)
	{
		vertexs[i].clear();
		indexes[i].clear();
		normals[i].clear();
	}
	if (pGenerator)
	{
		delete pGenerator;
		pGenerator = nullptr;
	}
	_initilized = false;
	_args.clear();
	_seed = 0;
}
NS_GNRT_END