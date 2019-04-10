#include "TerrianMesh.h"
#include "Logger/Logger.h"
using namespace G3D;
namespace generator {

	TerrianMesh::TerrianMesh() :
		m_bInitilized(false),
		m_pTerrianData(nullptr),
		m_pTopNeighbor(nullptr),
		m_pRightNeighbor(nullptr),
		m_pLeftNeighbor(nullptr),
		m_pBottomNeighbor(nullptr),
		m_cbMeshInitilizer(nullptr),
		m_pGenerator(nullptr),
		m_nSize(0)
	{
	}

	TerrianMesh::~TerrianMesh()
	{
		Release();
	}

	void TerrianMesh::Init(int32_t * args, int32_t argsize, MeshInitilizerCallBack callback, GeneratorNotifier notifier)
	{
		if (!callback)
		{
			LogError("TerrianDataBinding and MeshInitilizerCallBack cant be null");
			return;
		}
		m_cbMeshInitilizer = callback;
		m_cbNotifier = notifier;
		if (argsize < 7)
		{
			LogErrorFormat("Terrian Mesh Init fail,argsize %d less than 7", argsize);
			return;
		}
		m_pTerrianData = new TerrianDataBinding(args[1], args[2], args[argsize - 1] != 0);
		m_vInitilizeArgs.resize(argsize);
		memcpy(m_vInitilizeArgs.data(), args, sizeof(int32_t)*argsize);
		m_bInitilized = true;
	}
	void TerrianMesh::GetTerrianVerticesData(G3D::Vector3 * pV, G3D::Vector3* pN, int32_t size, int32_t mesh)
	{
		if (!m_bInitilized)
		{
			return;
		}
		if (m_pGenerator)
		{
			m_pGenerator->SetVerticesAndNormal(pV, pN, mesh);
		}
		else
		{
			LogErrorFormat("Generator %dwas released", this);
		}
		//m_terrianData->SetTerrianVerticesData(p, size, mesh);

	}
	void TerrianMesh::GetMeshUVData(G3D::Vector2 * p, int32_t size, int32_t mesh, int32_t uv)
	{
		if (!m_bInitilized)
		{
			return;
		}
		//m_terrianData->SetMeshUVData(p, size, mesh, uv);
	}
	void TerrianMesh::InitMeshTriangleData(int32_t* p, int32_t size, int32_t mesh, int32_t lod)
	{
		if (!m_bInitilized)
		{
			return;
		}
		RecaculateTriangles(p, size, mesh, lod);
	}
	void TerrianMesh::InitEdge(int32_t edge, TerrianMesh * mesh)
	{
		switch (edge)
		{
		case 0:
			m_pLeftNeighbor = mesh;
			break;;
		case  1:
			m_pRightNeighbor = mesh;
			break;;
		case 2:
			m_pBottomNeighbor = mesh;
			break;;
		case  3:
			m_pTopNeighbor = mesh;
			break;
		default:
			break;
		}
	}
	void TerrianMesh::RecaculateTriangles(int32_t* p, int32_t size, int32_t mesh, int32_t lod)
	{
		//simply caculate lod of zero and has no neighbors

		int nMax = m_nSize - 1;
		int	 dy = nMax / m_pTerrianData->meshCount;//vertice count per mesh
		int	 outBoundY((mesh + 1)*dy); //up bound
		int triangleIdx = 0;
		int startY(mesh*dy);//start bound
		int32_t* triangle = p;

		for (int ti = 0, vi = 0, y = startY; y < outBoundY && y < nMax; y++, vi++)
		{
			for (int x = 0; x < nMax; x++, ti += 6, vi++)
			{
				int p0 = vi;
				int p1 = vi + 1;
				int p2 = vi + nMax + 1;
				int p3 = vi + nMax + 2;
				triangle[triangleIdx++] = p0;
				triangle[triangleIdx++] = p2;
				triangle[triangleIdx++] = p1;
				triangle[triangleIdx++] = p1;
				triangle[triangleIdx++] = p2;
				triangle[triangleIdx++] = p3;
			}
		}
		mesh++;
		if (mesh < m_pTerrianData->meshCount)
		{
			m_cbNotifier(meshTopologyTriangle, mesh, lod);
		}
	}
	int32_t TerrianMesh::GetTriangleCount(int32_t mesh)
	{
		if (m_pTerrianData->currentLod == 0 || true)
		{
			if (m_pTerrianData->triangleSize[mesh] <= 0)
			{
				size_t meshCount = m_pTerrianData->meshCount;
				int idx = 0;
				int startY(0);
				int nMax = m_nSize - 1;
				int	 outBoundY(nMax);
				int obY(nMax);
				outBoundY = obY = nMax / meshCount;
				//int32_t*** triangles;
				while (idx < meshCount)
				{
					m_pTerrianData->triangleSize[idx] = nMax * (outBoundY - startY) * 6;
					LogFormat("triangle of mesh %d count %d",idx,m_pTerrianData->triangleSize[idx]);
					startY = outBoundY;//因为最上面和最右边一排不计算三角形，所以在交界处需要多计算一次
					outBoundY += obY;
					if (outBoundY >=nMax)
					{
						outBoundY = nMax;
					}
					idx++;
				}
				return m_pTerrianData->triangleSize[mesh];
			}
			else
			{
				return m_pTerrianData->triangleSize[mesh];
			}
		}
		else
		{
			return 0;
		}
	}
	void TerrianMesh::Start()
	{
		if (!m_bInitilized)
		{
			LogErrorFormat("terrian mesh not initialized yet!");
			return;
		}
		//std::thread t(std::bind(&TerrianMesh::WorkThread, this));
		//t.detach();
		WorkThread();
	}
	void TerrianMesh::Release()
	{
		safe_delete(m_pGenerator);
		m_vInitilizeArgs.clear();
	}
	void TerrianMesh::OnVerticesGenerateOver()
	{
	}
	void TerrianMesh::WorkThread()
	{
		m_pGenerator = new Diamond_Square(m_vInitilizeArgs[0], m_vInitilizeArgs[3], m_vInitilizeArgs[4], m_vHeightMap);
		float cornor[] = { m_vInitilizeArgs[6],m_vInitilizeArgs[7],m_vInitilizeArgs[8],m_vInitilizeArgs[9] };
		if (m_pLeftNeighbor)
		{
			//add left edge
		}
		if (m_pRightNeighbor)
		{
			//add right edge
		}
		if (m_pBottomNeighbor)
		{
			//add bottom edge
		}
		if (m_pTopNeighbor)
		{
			//add top edge
		}
		m_nSize = m_pGenerator->GetSquareSize();
		m_pGenerator->Start(cornor, 4);
		for (int i = 0;i<m_pTerrianData->meshCount;i++)
		{
			m_cbMeshInitilizer(meshTopologyVertice, i, m_pTerrianData->currentLod, m_pGenerator->GetVerticesSize(i));
		}
		m_pGenerator->GenerateTerrian(m_vInitilizeArgs[5]);

		m_cbNotifier(meshTopologyVertice, 0, m_pTerrianData->currentLod);
		GetTriangleCount(0);
		for (int i = 0; i < m_pTerrianData->meshCount; i++)
		{
			m_cbMeshInitilizer(meshTopologyTriangle, i, m_pTerrianData->currentLod, m_pTerrianData->triangleSize[i]);
		}
		m_cbNotifier(meshTopologyTriangle, 0, m_pTerrianData->currentLod);
	}
};