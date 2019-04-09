#ifndef TERRIAN_MESH_H
#define TERRIAN_MESH_H
#include "Generators/generator.h"
#include "Generators/TerrianGenerator/Diamond_Square.h"
NS_GNRT_START

class TerrianMesh
{
public:
	TerrianMesh();
	~TerrianMesh();
	void Init(int32_t* args ,int32_t argsize, ResizeIndicesCallBack callback);
	void InitTerrianVerticesData(G3D::Vector3* p, int32_t size, int32_t mesh);
	void InitMeshNormalData( G3D::Vector3* p, int32_t size, int32_t mesh);
	void InitMeshUVData( G3D::Vector2* p, int32_t size, int32_t mesh, int32_t uv);
	void InitMeshTriangleData(int32_t* p, int32_t size, int32_t mesh, int32_t lod);
	void InitEdge(int32_t edge,TerrianMesh* mesh);
	void RecaculateTriangles(int lod);
	void Start();
	void Release();
	int32_t GetLod() { return m_nLod; }
	void SetLod(int32_t lod) { m_nLod = lod; }
private: 
	void OnVerticesGenerateOver();
	void WorkThread();
private:
	TerrianDataBinding* m_terrianData;
	ResizeIndicesCallBack m_cbResizeIndices;
	TerrianMesh* m_pLeftNeighbor;
	TerrianMesh* m_pRightNeighbor;
	TerrianMesh* m_pBottomNeighbor;
	TerrianMesh* m_pTopNeighbor;
	std::vector<int32_t> m_vInitilizeArgs;
	std::vector<float> m_vHeightMap;
	int32_t m_nSize;
	bool m_bInitilized;
	int32_t m_nLod;
};
NS_GNRT_END
#endif
