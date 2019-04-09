#include "TerrianMesh.h"
#include "Logger/Logger.h"
using namespace G3D;
namespace generator {

	TerrianMesh::TerrianMesh() :
		m_bInitilized(false),
		m_terrianData(nullptr),
		m_pTopNeighbor(nullptr),
		m_pRightNeighbor(nullptr),
		m_pLeftNeighbor(nullptr),
		m_pBottomNeighbor(nullptr),
		m_cbResizeIndices(nullptr)
	{
	}

	TerrianMesh::~TerrianMesh()
	{
		Release();
	}

	void TerrianMesh::Init(int32_t * args, int32_t argsize, ResizeIndicesCallBack callback)
	{
		if (!callback)
		{
			LogError("TerrianDataBinding and ResizeIndicesCallBack cant be null");
			return;
		}
		m_cbResizeIndices = callback;
		if (argsize < 7)
		{
			LogErrorFormat("Terrian Mesh Init fail,argsize %d less than 7", argsize);
			return;
		}
		m_terrianData = new TerrianDataBinding(args[1], args[2], args[argsize - 1] != 0);
		m_vInitilizeArgs.resize(argsize);
		memcpy(m_vInitilizeArgs.data(), args, sizeof(int32_t)*argsize);
		m_bInitilized = true;
	}
	void TerrianMesh::InitTerrianVerticesData(G3D::Vector3 * p, int32_t size, int32_t mesh)
	{
		if (!m_bInitilized)
		{
			return;
		}
		m_terrianData->SetTerrianVerticesData(p, size, mesh);
	}
	void TerrianMesh::InitMeshNormalData(G3D::Vector3 * p, int32_t size, int32_t mesh)
	{
		if (!m_bInitilized)
		{
			return;
		}
		m_terrianData->SetMeshNormalData(p, size, mesh);
	}
	void TerrianMesh::InitMeshUVData(G3D::Vector2 * p, int32_t size, int32_t mesh, int32_t uv)
	{
		if (!m_bInitilized)
		{
			return;
		}
		m_terrianData->SetMeshUVData(p, size, mesh, uv);
	}
	void TerrianMesh::InitMeshTriangleData(int32_t * p, int32_t size, int32_t mesh, int32_t lod)
	{
		if (!m_bInitilized)
		{
			return;
		}
		m_terrianData->SetMeshTriangleData(p, size, mesh, lod);
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
	void TerrianMesh::RecaculateTriangles(int lod)
	{
		//simply caculate lod of zero and has no neighbors

		size_t meshCount = m_terrianData->meshCount;
		int idx = 0;
		int startY(0);
		int nMax = m_nSize - 1;
		int	 outBoundY(nMax);
		int obY(nMax);
		outBoundY = obY = nMax / meshCount;
		//int32_t*** triangles;
		while (idx < meshCount)
		{
			int triangleIdx = 0;
			int32_t* triangle = m_terrianData->triangles[idx][lod];
			m_cbResizeIndices(meshTopologyTriangle, idx, 0, nMax*(outBoundY - startY) * 6);
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
			//LogFormat("mesh %d v size %d,triangle size %d", idx, v3[idx].size(), triangles[idx].size());
			startY = outBoundY;//因为最上面和最右边一排不计算三角形，所以在交界处需要多计算一次
			outBoundY += obY;
			idx++;
		}
	}
	void TerrianMesh::Start()
	{
		if (!m_bInitilized)
		{
			LogErrorFormat("terrian mesh not initialized yet!");
			return;
		}
		std::thread t(std::bind(&TerrianMesh::WorkThread, this));
		t.detach();
	}
	void TerrianMesh::Release()
	{
		m_vInitilizeArgs.clear();
	}
	void TerrianMesh::OnVerticesGenerateOver()
	{
	}
	void TerrianMesh::WorkThread()
	{
		auto m_pGenerator = new Diamond_Square(m_vInitilizeArgs[0], m_vInitilizeArgs[3], m_vInitilizeArgs[4], m_vHeightMap);
		m_nSize = m_pGenerator->GetSquareSize();
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
		m_pGenerator->Start(cornor, 4);
		m_pGenerator->GenerateTerrian(m_terrianData->vertices, m_terrianData->normals, m_vInitilizeArgs[5], m_cbResizeIndices);
		delete m_pGenerator;
	}
};