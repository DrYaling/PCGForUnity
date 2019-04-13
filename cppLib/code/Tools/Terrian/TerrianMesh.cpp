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
#if TERRAIN_GENERATE_VERTICES
		m_cbNotifier = nullptr;
#endif
		m_cbMeshInitilizer = nullptr;
		LogFormat("TerrianMesh deleted");
	}


#if TERRAIN_GENERATE_VERTICES
	void TerrianMesh::Init(int32_t * args, int32_t argsize, MeshInitilizerCallBack callback, GeneratorNotifier notifier)
#else
	void TerrianMesh::Init(int32_t * args, int32_t argsize, MeshInitilizerCallBack callback)
#endif
	{
		if (!callback)
		{
			LogError("TerrianDataBinding and MeshInitilizerCallBack cant be null");
			return;
		}
		m_cbMeshInitilizer = callback;
#if TERRAIN_GENERATE_VERTICES
		m_cbNotifier = notifier;
#endif
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
#if TERRAIN_GENERATE_VERTICES
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
#endif
	void TerrianMesh::InitNeighbor(NeighborType edge, TerrianMesh * neighbor)
	{
		LogFormat("InitNeighbor %d,neighbor %d", edge, neighbor->m_nInstanceId);
		switch (edge)
		{
		case NeighborType::neighborPositionLeft:
			m_pLeftNeighbor = neighbor;
			if (neighbor && neighbor->m_pRightNeighbor != nullptr && neighbor->m_pRightNeighbor != this)
			{
				LogErrorFormat("fail to set neighbor ,left neighbor's right neighbor is not self");
				m_pLeftNeighbor = nullptr;
				return;
			}
			break;
		case  NeighborType::neighborPositionRight:
			m_pRightNeighbor = neighbor;
			if (neighbor && neighbor->m_pLeftNeighbor != nullptr && neighbor->m_pLeftNeighbor != this)
			{
				LogErrorFormat("fail to set neighbor ,right neighbor's left neighbor is not self");
				m_pRightNeighbor = nullptr;
				return;
			}
			break;
		case NeighborType::neighborPositionBottom:
			m_pBottomNeighbor = neighbor;
			if (neighbor && neighbor->m_pTopNeighbor != nullptr && neighbor->m_pTopNeighbor != this)
			{
				LogErrorFormat("fail to set neighbor ,bottom neighbor's top neighbor is not self");
				m_pBottomNeighbor = nullptr;
				return;
			}
			break;
		case  NeighborType::neighborPositionTop:
			m_pTopNeighbor = neighbor;
			if (neighbor && neighbor->m_pBottomNeighbor != nullptr && neighbor->m_pBottomNeighbor != this)
			{
				LogErrorFormat("fail to set neighbor ,top neighbor's bottom neighbor is not self");
				m_pTopNeighbor = nullptr;
				return;
			}
			break;
		default:
			break;
		}
#if TERRAIN_GENERATE_VERTICES
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
					LogFormat("mesh %dInitNeighbor %d", m_nInstanceId, i);
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
#endif
	}

	void TerrianMesh::InitVerticesWithNeighbor(NeighborType position)
	{
		float deltaSize = m_vInitilizeArgs[mesh_arg_mapWidth] / (float)(m_nSize - 1);
		int nMax = m_nSize - 1;
		bool insert = position == NeighborType::neighborPositionAll;
		//LogFormat("InitVerticesWithNeighbor pos %d,insert %d", position, insert);
		if (insert || position == NeighborType::neighborPositionLeft)
		{
			if (m_pLeftNeighbor)
			{
				//LogFormat("init left pulse ");
				if (m_pLeftNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pLeftNeighbor->m_nSize);
					return;
				}
				else if (m_pLeftNeighbor->m_vHeightMap.size() > 0)
				{
					//add left edge
					int x = 0;
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

		if (insert || position == NeighborType::neighborPositionRight)
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
					int x = nMax;
					int nx(x - nMax);
					for (int y = 0; y <= nMax; y++)
					{
						float nheight = m_pRightNeighbor->m_vHeightMap[nx + y * m_nSize];
						m_pGenerator->SetPulse(x, y, x*deltaSize, nheight, y*deltaSize, insert);
					}
				}
			}
		}

		if (insert || position == NeighborType::neighborPositionBottom)
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
					int y = 0;
					int nY(y + nMax);
					for (int x = 0; x <= nMax; x++)
					{
						float nheight = m_pBottomNeighbor->m_vHeightMap[x + nY * m_nSize];
						m_pGenerator->SetPulse(x, y, x*deltaSize, nheight, y*deltaSize, insert);
					}
				}
			}
		}

		if (insert || position == NeighborType::neighborPositionTop)
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
					int y = nMax;
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
#if TERRAIN_GENERATE_VERTICES
	void TerrianMesh::OnNeighborLodChanged(TerrianMesh* neighbor)
	{
		if (!neighbor)
			return;

		//first step ,exclude unnecessary vertice close to neighbor
		//second step ,recaculate triangles
		if (neighbor == m_pLeftNeighbor)
		{

		}
		else if (neighbor == m_pRightNeighbor)
		{

		}
		else if (neighbor == m_pBottomNeighbor)
		{
		}
		else if (neighbor == m_pTopNeighbor)
		{

		}
	}
	void TerrianMesh::RecaculateTriangles(int32_t* p, int32_t size, int32_t mesh, int32_t lod)
	{
		//simply caculate lod of zero and has no neighbors
		/*
		for those lod > 0
		//first step ,exclude unnecessary vertice
		//second step ,recaculate triangles
		*/
		int nMax = m_nSize - 1;
		int triangleIdx = 0;
		int32_t* triangle = p;
		const triangleSize_t& ts = m_pTerrianData->triangleSize[mesh];
		for (int ti = 0, vi = 0, y = ts.lowBound; y < ts.highBound && y < nMax; y += ts.unit, vi += ts.unit)
		{
			int unitY = ts.unit;
			if (y == nMax - ts.extUnitX)
			{
				unitY = ts.extUnitX;
			}
			for (int x = 0; x < nMax && triangleIdx < size; x += ts.unit, vi += ts.unit, ti += 6)
			{
				int unitX = ts.unit;
				if (x == nMax - ts.extUnitX)
				{
					unitX = ts.extUnitX;
				}
				int p0 = vi;
				int p1 = vi + unitX;
				int p2 = vi + nMax * unitY + 1;
				int p3 = p2 + unitX;
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
	//caculate triangle count with the effection of neighbors on given lod
	int32_t TerrianMesh::GetTriangleCount(int32_t mesh, int32_t lod)
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
				triangleSize_t& tsinfo = m_pTerrianData->triangleSize[idx];
				tsinfo.unit = lod + 1;
				int sizeY = outBoundY - startY;
				int xSize = nMax % tsinfo.unit + nMax / tsinfo.unit;
				int ySize = sizeY % tsinfo.unit + sizeY / tsinfo.unit;
				tsinfo.extUnitX = nMax % tsinfo.unit;
				tsinfo.extUnitY = ySize % tsinfo.unit;
				tsinfo.size = xSize * ySize * 6;
				tsinfo.lowBound = startY;
				tsinfo.highBound = outBoundY;
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
#endif
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
			LogFormat("Generator size %d", m_pGenerator->GetAlloc());
		}
		//release_vector(m_vInitilizeArgs, int32_t);

		LogFormat("TerrianMesh %d Released", m_nInstanceId);
	}
	void TerrianMesh::GetHeightMap(float * heightMap, int32_t size1, int32_t size2)
	{
		if (heightMap)
		{
			if (size1 == size2 && size1 == m_nSize)
			{
				//memcpy(heightMap, m_vHeightMap.data(), m_vHeightMap.size()*sizeof(float));
				for (int32_t i = 0; i< m_vHeightMap.size(); i++)
					heightMap[i] = m_vHeightMap[i] / MAX_MAP_HEIGHT;
				/*for (int32_t x = 0; x < size1; x++)
				{
					for (int32_t y = 0; y < size2; y++)
					{
						//LogFormat("heightMap at x %d,y %d,idx %d", x, y, x + y * m_nSize);
						heightMap[x + y * m_nSize] = m_vHeightMap[x + y * m_nSize] / MAX_MAP_HEIGHT;
					}
				}*/
			}
			else
			{
				LogErrorFormat("GetHeightMap Fail size x %d,size y %d,size height map %d", size1, size2, m_nSize);
			}
		}
		else
		{
			LogErrorFormat("GetHeightMap Fail with null map ptr");
		}
	}
#if TERRAIN_GENERATE_VERTICES
	bool TerrianMesh::GetNeighborVertice(int32_t x, int32_t y, int32_t neighbor, G3D::Vector3 & p)
#else
	bool TerrianMesh::GetNeighborVertice(int32_t x, int32_t y, NeighborType neighbor, float & p)
#endif
	{
		switch (neighbor)
		{
		case NeighborType::neighborPositionLeft:
			//prevent unlimited circle
			if (m_pLeftNeighbor && m_pLeftNeighbor->m_bGenerated/* && (m_pLeftNeighbor->m_pRightNeighbor == nullptr || m_pLeftNeighbor->m_pRightNeighbor == this)*/)
			{
				//if (x < m_nSize && x >= 0)
				//{
					//LogFormat("%d,%d,x %d,y %d",m_pLeftNeighbor,m_pLeftNeighbor->m_pGenerator,x,y);
#if TERRAIN_GENERATE_VERTICES
				p = m_pLeftNeighbor->m_pGenerator->GetRealVertice(x, y);
#else
				p = m_pLeftNeighbor->m_pGenerator->GetAtXY(x, y);
#endif
				return true;
				//}
			}
			break;
		case  NeighborType::neighborPositionRight:
			if (m_pRightNeighbor && m_pRightNeighbor->m_bGenerated /*&& (m_pRightNeighbor->m_pLeftNeighbor == nullptr || m_pRightNeighbor->m_pLeftNeighbor == this)*/)
			{
				//if (x >= 0 && x < m_nSize)
				//{
#if TERRAIN_GENERATE_VERTICES
				p = m_pRightNeighbor->m_pGenerator->GetRealVertice(x, y);
#else
				p = m_pRightNeighbor->m_pGenerator->GetAtXY(x, y);
#endif
				//LogFormat("GetNeighborVertice at x %d,y %d,h %f", x, y, p.y);
				return true;
				//}
				/*else
				{
					LogFormat("GetNeighborVertice at x %d,y %d,fail", x, y);

				}*/
			}
			break;
		case NeighborType::neighborPositionBottom:
			if (m_pBottomNeighbor && m_pBottomNeighbor->m_bGenerated /*&& (m_pBottomNeighbor->m_pTopNeighbor == nullptr || m_pBottomNeighbor->m_pTopNeighbor == this)*/)
			{
				//if (y >= 0 && y < m_nSize)
				//{
#if TERRAIN_GENERATE_VERTICES
				p = m_pBottomNeighbor->m_pGenerator->GetRealVertice(x, y);
#else
				p = m_pBottomNeighbor->m_pGenerator->GetAtXY(x, y);
#endif
				return true;
				//}
			}
			break;
		case  NeighborType::neighborPositionTop:
			if (m_pTopNeighbor && m_pTopNeighbor->m_bGenerated /*&& (m_pTopNeighbor->m_pBottomNeighbor == nullptr || m_pTopNeighbor->m_pBottomNeighbor == this)*/)
			{
				//if (y >= 0 && y < m_nSize)
				//{
#if TERRAIN_GENERATE_VERTICES
				p = m_pTopNeighbor->m_pGenerator->GetRealVertice(x, y);
#else
				p = m_pTopNeighbor->m_pGenerator->GetAtXY(x, y);
#endif
				return true;
				//}
			}
			break;
		default:
			break;
		}
		return false;
	}
	void TerrianMesh::WorkThread()
	{
		m_pGenerator = new Diamond_Square(m_vInitilizeArgs[mesh_arg_seed], m_vInitilizeArgs[mesh_arg_I], m_vInitilizeArgs[mesh_arg_H], m_vHeightMap);
		auto callback = std::bind(&TerrianMesh::GetNeighborVertice, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		m_pGenerator->SetGetVerticeCallBack(callback);
#if TERRAIN_GENERATE_VERTICES
		m_pTerrianData->SetMeshCount(m_pGenerator->GetMeshRealCount(), m_pGenerator->GetMeshTheoreticalCount());
#endif
		m_cbMeshInitilizer(m_nInstanceId, meshTopologyMeshCount, m_pTerrianData->meshCount, 0, 0);
		m_nSize = m_pGenerator->GetSquareSize();
		float cornor[] = { m_vInitilizeArgs[mesh_arg_h0],m_vInitilizeArgs[mesh_arg_h1],m_vInitilizeArgs[mesh_arg_h2],m_vInitilizeArgs[mesh_arg_h3] };

		InitVerticesWithNeighbor();
		m_pGenerator->Start(cornor, 4);
#if TERRAIN_GENERATE_VERTICES
		for (int i = 0; i < m_pTerrianData->meshCount; i++)
		{
			m_cbMeshInitilizer(m_nInstanceId, meshTopologyVertice, i, m_pTerrianData->currentLod, m_pGenerator->GetVerticesSize(i));
		}
		m_pGenerator->GenerateTerrian(m_vInitilizeArgs[mesh_arg_mapWidth]);
		LogFormat("Generator size %d", m_pGenerator->GetAlloc());

		for (int i = 0; i < m_pTerrianData->meshCount; i++)
		{
			m_cbNotifier(m_nInstanceId, meshTopologyVertice, i, m_pTerrianData->currentLod);
		}
		GetTriangleCount(0, 0);
		for (int i = 0; i < m_pTerrianData->meshCount; i++)
		{
			m_cbMeshInitilizer(m_nInstanceId, meshTopologyTriangle, i, m_pTerrianData->currentLod, m_pTerrianData->triangleSize[i].size);
		}
		m_cbNotifier(m_nInstanceId, meshTopologyTriangle, 0, m_pTerrianData->currentLod);
#else
		m_cbMeshInitilizer(m_nInstanceId, (int32_t)TerrainInitType::HeightMap, m_nSize, m_nSize, m_vHeightMap.size());
#endif
	}
};