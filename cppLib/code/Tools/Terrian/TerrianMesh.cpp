#include "TerrianMesh.h"
#include "Logger/Logger.h"
using namespace G3D;
namespace generator {

	TerrainMesh::TerrainMesh(int32_t ins) :
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

	TerrainMesh::~TerrainMesh()
	{
		Release();
		safe_delete(m_pGenerator);
		m_cbMeshInitilizer = nullptr;
		//LogFormat("TerrainMesh deleted");
	}

	bool TerrainMesh::StaticGetNeighborVertice(int32_t x, int32_t y, NeighborType neighbor, float & p,void* owner)
	{
		TerrainMesh* mesh = static_cast<TerrainMesh*>(owner);
		if (mesh)
		{
			return mesh->GetNeighborVertice(x, y, neighbor, p);
		}
		else
		{
			return false;
		}
	}

	void TerrainMesh::Init(int32_t * args, int32_t argsize, float* heightMap, int32_t heightMapSize, MeshInitilizerCallBack callback)
	{
		if (!callback)
		{
			LogError("TerrianDataBinding and MeshInitilizerCallBack cant be null");
			return;
		}
		if (!heightMap)
		{
			LogError("TerrianDataBinding and heightMap cant be null");
			return;
		}
		m_vHeightMap = heightMap;
		m_nheightMapSize = heightMapSize;
		m_cbMeshInitilizer = callback;
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

	void TerrainMesh::InitNeighbor(NeighborType edge, TerrainMesh * neighbor)
	{
		//LogFormat("InitNeighbor %d,neighbor %d", edge, neighbor->m_nInstanceId);
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
	}

	void TerrainMesh::InitVerticesWithNeighbor(NeighborType position)
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
				else if (m_pLeftNeighbor->m_nheightMapSize > 0)
				{
					//add left edge
					int x = 0;
					int nx(x + nMax);
					for (int y = 0; y <= nMax; y++)
					{
						float nheight = m_pLeftNeighbor->m_vHeightMap[GetHeightMapIndex(nx, y)];
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
				else if (m_pRightNeighbor->m_nheightMapSize > 0)
				{
					int x = nMax;
					int nx(x - nMax);
					for (int y = 0; y <= nMax; y++)
					{
						float nheight = m_pRightNeighbor->m_vHeightMap[GetHeightMapIndex(nx, y)];
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
				else if (m_pBottomNeighbor->m_nheightMapSize > 0)
				{
					int y = 0;
					int nY(y + nMax);
					for (int x = 0; x <= nMax; x++)
					{
						float nheight = m_pBottomNeighbor->m_vHeightMap[GetHeightMapIndex(x, nY)];
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
				else if (m_pTopNeighbor->m_nheightMapSize > 0)
				{
					int y = nMax;
					int nY(y - nMax);
					for (int x = 0; x <= nMax; x++)
					{
						float nheight = m_pTopNeighbor->m_vHeightMap[GetHeightMapIndex(x, nY)];
						m_pGenerator->SetPulse(x, y, x*deltaSize, nheight, y*deltaSize, insert);
					}
				}
			}
		}
	}
	void TerrainMesh::Start()
	{
		if (!m_bInitilized)
		{
			LogErrorFormat("terrian neighbor not initialized yet!");
			return;
		}
		WorkThread();
	}
	void TerrainMesh::Release()
	{
		if (m_pGenerator)
		{
			m_pGenerator->ReleaseUnusedBuffer();
		}
	}
	void TerrainMesh::GetHeightMap(float * heightMap, int32_t size1, int32_t size2)
	{
		if (heightMap)
		{
			if (size1 == size2 && size1 == m_nSize)
			{
				//memcpy(heightMap, m_vHeightMap.data(), m_vHeightMap.size()*sizeof(float));
				for (int32_t i = 0; i < m_nheightMapSize; i++)
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
	bool TerrainMesh::GetNeighborVertice(int32_t x, int32_t y, NeighborType neighbor, float & p)
	{
		switch (neighbor)
		{
		case NeighborType::neighborPositionLeft:
			//prevent unlimited circle
			if (m_pLeftNeighbor && m_pLeftNeighbor->m_bGenerated/* && (m_pLeftNeighbor->m_pRightNeighbor == nullptr || m_pLeftNeighbor->m_pRightNeighbor == this)*/)
			{
				p = m_pLeftNeighbor->m_pGenerator->GetAtXY(x, y);
				return true;
			}
			break;
		case  NeighborType::neighborPositionRight:
			if (m_pRightNeighbor && m_pRightNeighbor->m_bGenerated /*&& (m_pRightNeighbor->m_pLeftNeighbor == nullptr || m_pRightNeighbor->m_pLeftNeighbor == this)*/)
			{
				p = m_pRightNeighbor->m_pGenerator->GetAtXY(x, y);
				return true;
			}
			break;
		case NeighborType::neighborPositionBottom:
			if (m_pBottomNeighbor && m_pBottomNeighbor->m_bGenerated /*&& (m_pBottomNeighbor->m_pTopNeighbor == nullptr || m_pBottomNeighbor->m_pTopNeighbor == this)*/)
			{
				p = m_pBottomNeighbor->m_pGenerator->GetAtXY(x, y);
				return true;
			}
			break;
		case  NeighborType::neighborPositionTop:
			if (m_pTopNeighbor && m_pTopNeighbor->m_bGenerated /*&& (m_pTopNeighbor->m_pBottomNeighbor == nullptr || m_pTopNeighbor->m_pBottomNeighbor == this)*/)
			{
				p = m_pTopNeighbor->m_pGenerator->GetAtXY(x, y);
				return true;
			}
			break;
		default:
			break;
		}
		return false;
	}
	void TerrainMesh::WorkThread()
	{
		m_pGenerator = new Diamond_Square(m_vInitilizeArgs[mesh_arg_seed], m_vInitilizeArgs[mesh_arg_I], m_vInitilizeArgs[mesh_arg_H], m_vHeightMap,this);
		//auto callback = std::bind(&TerrainMesh::GetNeighborVertice, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		m_pGenerator->SetGetVerticeCallBack(&TerrainMesh::StaticGetNeighborVertice);
		//m_cbMeshInitilizer(m_nInstanceId, meshTopologyMeshCount, m_pTerrianData->meshCount, 0, 0);
		m_nSize = m_pGenerator->GetSquareSize();
		float cornor[] = { m_vInitilizeArgs[mesh_arg_h0],m_vInitilizeArgs[mesh_arg_h1],m_vInitilizeArgs[mesh_arg_h2],m_vInitilizeArgs[mesh_arg_h3] };

		InitVerticesWithNeighbor();
		m_pGenerator->Start(cornor, 4);
		//LogErrorFormat("cpp height 0 %f,1 %f",m_vHeightMap[0], m_vHeightMap[m_nSize+1]);
		m_cbMeshInitilizer(m_nInstanceId, (int32_t)TerrainInitType::HeightMap, m_nSize, m_nSize, m_nheightMapSize);
	}
};