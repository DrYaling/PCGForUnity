#ifndef TERRIAN_MESH_H
#define TERRIAN_MESH_H
#include "generator.h"
#include "Generators/TerrainGenerator/Diamond_Square.h"
NS_GNRT_START

class Terrain
{
public:
	Terrain(int32_t ins);
	~Terrain();
	void Init(int32_t* args, int32_t argsize,float* heightMap,int32_t heightMapSize, MeshInitilizerCallBack callback);
	void InitVerticesWithNeighbor(NeighborType position = NeighborType::neighborPositionAll);
	void InitNeighbor(NeighborType edge, Terrain* mesh);
	void Start();
	void Release();
	void GetHeightMap(float* heightMap, int32_t size1, int32_t size2);
private:
	bool GetNeighborVertice(int32_t x, int32_t y, NeighborType neighbor, float & p);
	static bool StaticGetNeighborVertice(int32_t x, int32_t y, NeighborType neighbor, float & p,void* owner);
	void WorkThread();
private:
	TerrianDataBinding * m_pTerrianData;
	MeshInitilizerCallBack m_cbMeshInitilizer;
	Terrain* m_pLeftNeighbor;
	Terrain* m_pRightNeighbor;
	Terrain* m_pBottomNeighbor;
	Terrain* m_pTopNeighbor;
	Diamond_Square* m_pGenerator;
	std::vector<int32_t> m_vInitilizeArgs;
	/*std::vector<float>*/float* m_vHeightMap;
	int32_t m_nheightMapSize;
	int32_t m_nSize;
	bool m_bInitilized;
	bool m_bGenerated;
	int32_t m_nInstanceId;
};
NS_GNRT_END
#endif
