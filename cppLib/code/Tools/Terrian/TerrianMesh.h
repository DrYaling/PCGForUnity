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
	void Init(int32_t* args, int32_t argsize, MeshInitilizerCallBack callback, GeneratorNotifier notifier);
	void GetTerrianVerticesData(G3D::Vector3* pV, G3D::Vector3* pN, int32_t size, int32_t mesh);
	void GetMeshUVData(G3D::Vector2* p, int32_t size, int32_t mesh, int32_t uv);
	void InitMeshTriangleData(int32_t* p, int32_t size, int32_t mesh, int32_t lod);
	void InitEdge(int32_t edge, TerrianMesh* mesh);
	void RecaculateTriangles(int32_t* p, int32_t size, int32_t mesh, int32_t lod);
	int32_t GetTriangleCount(int32_t mesh);
	void Start();
	void Release();
	int32_t GetLod() { return m_pTerrianData->currentLod; }
	void SetLod(int32_t lod) {
		if (lod != m_pTerrianData->currentLod)
		{
			m_pTerrianData->currentLod = lod;
			for (int i = 0;i<m_pTerrianData->triangleSize.size();i++)
			{
				m_pTerrianData->triangleSize[i] = 0;
			}
			GetTriangleCount(0);
			for (int i = 0; i < m_pTerrianData->meshCount; i++)
			{
				m_cbMeshInitilizer(meshTopologyTriangle, i, m_pTerrianData->currentLod, m_pTerrianData->triangleSize[i]);
			}
		}
	}
private:
	void OnVerticesGenerateOver();
	void WorkThread();
private:
	TerrianDataBinding * m_pTerrianData;
	MeshInitilizerCallBack m_cbMeshInitilizer;
	GeneratorNotifier m_cbNotifier;
	TerrianMesh* m_pLeftNeighbor;
	TerrianMesh* m_pRightNeighbor;
	TerrianMesh* m_pBottomNeighbor;
	TerrianMesh* m_pTopNeighbor;
	Diamond_Square* m_pGenerator;
	std::vector<int32_t> m_vInitilizeArgs;
	std::vector<float> m_vHeightMap;
	int32_t m_nSize;
	bool m_bInitilized;
};
NS_GNRT_END
#endif