#include "MeshGenerator.h"
#include "Logger/Logger.h"
NS_GNRT_START

MountainGen* mtg = nullptr;
std::vector<Vector3> vertexs;
std::vector<int32_t> indexes;
void InitGenerator(Vector3 pos, int32_t seed, int32_t depth, int32_t step,int32_t maxHeight, bool usePerlin)
{
	mtg = new MountainGen(std::move(pos), depth);
	mtg->Init(seed,step,maxHeight,usePerlin);
	vertexs.clear();
	indexes.clear();
}

void GenrateMountain(int32_t& vSize, int32_t& idxSize)
{
	if (mtg)
	{
		mtg->Start(indexes,vertexs);
		vSize = vertexs.size();
		idxSize = indexes.size();
	}
	else
	{
		vSize = idxSize = 0;
	}
}

void GetGeneratorData(Vector3 * pV, int32_t * pI)
{
	if (pV)
	{
		for (auto v :vertexs)
		{
			*pV++ = v;
		}
	}
	if (pI)
	{
		for (auto i: indexes)
		{
			*pI++ = i;
		}
	}
}

void ReleaseGenerator()
{
	for (auto itr = vertexs.begin(); itr != vertexs.end();)
	{
		itr = vertexs.erase(itr);
	}
	for (auto itr = indexes.begin(); itr != indexes.end();)
	{
		itr = indexes.erase(itr);
	}
	if (mtg)
	{
		delete mtg;
	}
}
NS_GNRT_END