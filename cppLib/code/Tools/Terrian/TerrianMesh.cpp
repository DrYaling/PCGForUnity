#include "TerrianMesh.h"
#include "Logger/Logger.h"
using namespace G3D;
namespace generator {

	TerrianMesh::TerrianMesh(int32_t ins) :
		m_bInitilized(false),
		m_bGenerated(false),
		m_pTerrianData(nullptr),
		m_pTopNeighbor(nullptr),
		m_pRightNeighbor(nullptr),
		m_pLeftNeighbor(nullptr),
		m_pBottomNeighbor(nullptr),
		m_cbMeshInitilizer(nullptr),
		m_pGenerator(nullptr),
		m_nInstanceId(ins),
		m_nSize(0)
	{
	}

	TerrianMesh::~TerrianMesh()
	{
		Release();
		safe_delete(m_pGenerator);
		m_cbNotifier = nullptr;
		m_cbMeshInitilizer = nullptr;
		LogFormat("TerrianMesh deleted");
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
		m_pTerrianData = new TerrianDataBinding(args[mesh_arg_lod], args[mesh_arg_useuv] != 0);
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
			m_pGenerator->SetVerticesAndNormal(pV, pN, size, mesh);
			m_bGenerated = true;
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
	void TerrianMesh::InitNeighbor(int32_t edge, TerrianMesh * neighbor, bool reloadNormalIfLoaded)
	{
		LogFormat("InitNeighbor %d,neighbor %d", edge, neighbor->m_nInstanceId);
		switch (edge)
		{
		case neighborPositionLeft:
			m_pLeftNeighbor = neighbor;
			break;
		case  neighborPositionRight:
			m_pRightNeighbor = neighbor;
			break;
		case neighborPositionBottom:
			m_pBottomNeighbor = neighbor;
			break;
		case  neighborPositionTop:
			m_pTopNeighbor = neighbor;
			break;
		default:
			break;
		}
		//if generated,recaculate normals of this mesh
		if (m_bGenerated && reloadNormalIfLoaded)
		{
			InitVerticesWithNeighbor(edge);
			switch (edge)
			{
			case neighborPositionLeft:
			case  neighborPositionRight:
				for (int i = 0; i < m_pTerrianData->meshCount; i++)
				{
					LogFormat("InitNeighbor %d", i);
					m_cbNotifier(m_nInstanceId, meshTopologyNormal, i, edge);
				}
			case neighborPositionBottom:
				m_cbNotifier(m_nInstanceId, meshTopologyNormal, 0, edge);
				break;
			case  neighborPositionTop:
				m_cbNotifier(m_nInstanceId, meshTopologyNormal, m_pTerrianData->meshCount - 1, edge);
				break;
			default:
				break;
			}
		}
	}
	void TerrianMesh::RecaculateTriangles(int32_t* p, int32_t size, int32_t mesh, int32_t lod)
	{
		//simply caculate lod of zero and has no neighbors

		int nMax = m_nSize - 1;
		int triangleIdx = 0;
		int32_t* triangle = p;
		triangleSize_t ts = m_pTerrianData->triangleSize[mesh];
		for (int ti = 0, vi = 0, y = ts.lowBound; y < ts.highBound && y < nMax; y++, vi++)
		{
			for (int x = 0; x < nMax && triangleIdx < size; x++, ti += 6, vi++)
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
			m_cbNotifier(m_nInstanceId, meshTopologyTriangle, mesh, lod);
		}
	}
	void TerrianMesh::RecaculateNormal(G3D::Vector3 * pN, int32_t size, int32_t mesh, int32_t position)
	{
		if (m_pGenerator)
		{
			m_pGenerator->RecaculateNormal(pN, size, mesh, position);
		}
		else
		{
			LogErrorFormat("Terrian Mesh was not loaded yet!");
		}
	}
	void TerrianMesh::InitVerticesWithNeighbor(int32_t position)
	{
		float deltaSize = m_vInitilizeArgs[mesh_arg_mapWidth] / (float)(m_nSize - 1);
		int nMax = m_nSize - 1;
		bool insert = position == neighborPositionAll;
		LogFormat("InitVerticesWithNeighbor pos %d,insert %d", position, insert);
		if (insert || position == neighborPositionLeft)
		{
			if (m_pLeftNeighbor)
			{
				LogFormat("init left pulse ");
				if (m_pLeftNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pLeftNeighbor->m_nSize);
					return;
				}
				else if (m_pLeftNeighbor->m_vHeightMap.size() > 0)
				{
					//add left edge
					for (int x = -1; x <= 0; x++)
					{
						int nx(x + nMax);
						for (int y = 0; y <= nMax; y++)
						{
							float nheight = m_pLeftNeighbor->m_vHeightMap[nx + y * m_nSize];
							m_pGenerator->SetPulse(x, y, x*deltaSize, nheight, y*deltaSize, insert);
							//LogFormat("SetPulse x %d,y %d, nx %d,height %f", x, y, nx, nheight);
						}
					}
				}
			}
		}

		if (insert || position == neighborPositionRight)
		{
			if (m_pRightNeighbor)
			{
				//add right edge
				if (m_pRightNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pRightNeighbor->m_nSize);
					return;
				}
				else if (m_pRightNeighbor->m_vHeightMap.size() > 0)
				{
					for (int x = nMax; x <= m_nSize; x++)
					{
						int nx(x - nMax);
						for (int y = 0; y <= nMax; y++)
						{
							float nheight = m_pRightNeighbor->m_vHeightMap[nx + y * m_nSize];
							m_pGenerator->SetPulse(x, y, x*deltaSize, nheight, y*deltaSize, insert);
						}
					}
				}
			}
		}

		if (insert || position == neighborPositionBottom)
		{
			if (m_pBottomNeighbor)
			{
				//add bottom edge
				if (m_pBottomNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pBottomNeighbor->m_nSize);
					return;
				}
				else if (m_pBottomNeighbor->m_vHeightMap.size() > 0)
				{
					for (int y = -1; y <= 0; y++)
					{
						int nY(y + nMax);
						for (int x = 0; x <= nMax; x++)
						{
							float nheight = m_pBottomNeighbor->m_vHeightMap[x + nY * m_nSize];
							m_pGenerator->SetPulse(x, y, x*deltaSize, nheight, y*deltaSize, insert);
						}
					}
				}
			}
		}

		if (insert || position == neighborPositionTop)
		{
			if (m_pTopNeighbor)
			{
				//add top edge
				if (m_pTopNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pTopNeighbor->m_nSize);
					return;
				}
				else if (m_pTopNeighbor->m_vHeightMap.size() > 0)
				{
					for (int y = nMax; y <= m_nSize; y++)
					{
						int nY(y - nMax);
						for (int x = 0; x <= nMax; x++)
						{
							float nheight = m_pTopNeighbor->m_vHeightMap[x + nY * m_nSize];
							m_pGenerator->SetPulse(x, y, x*deltaSize, nheight, y*deltaSize, insert);
						}
					}
				}
			}
		}
	}
	int32_t TerrianMesh::GetTriangleCount(int32_t mesh)
	{
		if (m_pTerrianData->currentLod == 0 || true)
		{
			if (m_pTerrianData->triangleSize[mesh].size <= 0)
			{
				size_t meshCount = m_pTerrianData->meshCount;
				int idx = 0;
				int startY(0);
				int nMax = m_nSize - 1;
				int	 outBoundY(nMax);
				int obY(nMax);
				outBoundY = obY = nMax / m_pTerrianData->theoreticalMeshCount;
				//int32_t*** triangles;
				while (idx < meshCount)
				{
					m_pTerrianData->triangleSize[idx].size = nMax * (outBoundY - startY) * 6;
					m_pTerrianData->triangleSize[idx].lowBound = startY;
					m_pTerrianData->triangleSize[idx].highBound = outBoundY;
					//LogFormat("triangle of mesh %d count %d,ob %d,st %d", idx, m_pTerrianData->triangleSize[idx].size, outBoundY, startY);
					startY = outBoundY;//因为最上面和最右边一排不计算三角形，所以在交界处需要多计算一次
					outBoundY += obY;
					if (outBoundY >= nMax)
					{
						outBoundY = nMax;
					}
					idx++;
				}
				return m_pTerrianData->triangleSize[mesh].size;
			}
			else
			{
				return m_pTerrianData->triangleSize[mesh].size;
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
			LogErrorFormat("terrian neighbor not initialized yet!");
			return;
		}
		/*std::thread t(std::bind(&TerrianMesh::WorkThread, this));
		t.detach();*/
		WorkThread();
	}
	void TerrianMesh::Release()
	{
		if (m_pGenerator)
		{
			m_pGenerator->ReleaseUnusedBuffer();
		}
		m_vInitilizeArgs.clear();
		LogFormat("TerrianMesh Released");
	}
	void TerrianMesh::OnVerticesGenerateOver()
	{
	}
	void TerrianMesh::WorkThread()
	{
		m_pGenerator = new Diamond_Square(m_vInitilizeArgs[mesh_arg_seed], m_vInitilizeArgs[mesh_arg_I], m_vInitilizeArgs[mesh_arg_H], m_vHeightMap);
		m_pTerrianData->SetMeshCount(m_pGenerator->GetMeshRealCount(), m_pGenerator->GetMeshTheoreticalCount());
		m_cbMeshInitilizer(m_nInstanceId, meshTopologyMeshCount, m_pTerrianData->meshCount, 0, 0);
		m_nSize = m_pGenerator->GetSquareSize();
		float cornor[] = { m_vInitilizeArgs[mesh_arg_h0],m_vInitilizeArgs[mesh_arg_h1],m_vInitilizeArgs[mesh_arg_h2],m_vInitilizeArgs[mesh_arg_h3] };
		InitVerticesWithNeighbor();
		m_pGenerator->Start(cornor, 4);
		for (int i = 0; i < m_pTerrianData->meshCount; i++)
		{
			m_cbMeshInitilizer(m_nInstanceId, meshTopologyVertice, i, m_pTerrianData->currentLod, m_pGenerator->GetVerticesSize(i));
		}
		m_pGenerator->GenerateTerrian(m_vInitilizeArgs[mesh_arg_mapWidth]);

		for (int i = 0; i < m_pTerrianData->meshCount; i++)
		{
			m_cbNotifier(m_nInstanceId, meshTopologyVertice, i, m_pTerrianData->currentLod);
		}
		GetTriangleCount(0);
		for (int i = 0; i < m_pTerrianData->meshCount; i++)
		{
			m_cbMeshInitilizer(m_nInstanceId, meshTopologyTriangle, i, m_pTerrianData->currentLod, m_pTerrianData->triangleSize[i].size);
		}
		m_cbNotifier(m_nInstanceId, meshTopologyTriangle, 0, m_pTerrianData->currentLod);
	}
};