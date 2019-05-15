#include "Terrain.h"
#include "Logger/Logger.h"
using namespace G3D;
namespace generator {

	Terrain::Terrain(uint32_t ins, uint32_t I, uint32_t heightMapSize) :
		m_pTopNeighbor(nullptr),
		m_pRightNeighbor(nullptr),
		m_pLeftNeighbor(nullptr),
		m_pBottomNeighbor(nullptr),
		m_nInstanceId(ins),
		m_aHeightMap(nullptr),
		m_nI(I),
		m_nSize(heightMapSize),
		m_nRealWidth(heightMapSize)
	{
		m_bHeightMapInitilized = false;
		/*switch (I)
		{
			case 1:
			case 2:
				m_nRealWidth = heightMapSize;
				break;
			case 3:
				m_nRealWidth = 64;
				break;
			case 4:
				m_nRealWidth = 128;
				break;
			case 5:
				m_nRealWidth = 512;
				break;
			case 6:
				m_nRealWidth = 2400;
				break;
			default:
				LogErrorFormat("Error MapSize %d not supported", I);
				return;
		}*/
	}

	/*Terrain::Terrain(const Terrain & t)
	{
		m_aHeightMap = t.m_aHeightMap;
		m_aSplatMap = t.m_aSplatMap;
		m_nInstanceId = t.m_nInstanceId;
		m_nSize = t.m_nSize;
	}

	Terrain::Terrain(const Terrain && t)
	{
		m_aHeightMap = t.m_aHeightMap;
		m_aSplatMap = t.m_aSplatMap;
		m_nInstanceId = t.m_nInstanceId;
		m_nSize = t.m_nSize;
	}*/

	Terrain::~Terrain()
	{
	}
	uint32_t Terrain::GetNeighbor(NeighborType neighbor) const
	{
		//LogFormat("get %d neighbor of %d", neighbor, m_nInstanceId);
		switch (neighbor)
		{
			case NeighborType::neighborPositionLeft:
				if (m_pLeftNeighbor)
				{
					return m_pLeftNeighbor->m_nInstanceId;
				}
				break;
			case NeighborType::neighborPositionBottom:
				if (m_pBottomNeighbor)
				{
					return m_pBottomNeighbor->m_nInstanceId;
				}
				break;
			case NeighborType::neighborPositionRight:
				if (m_pRightNeighbor)
				{
					return m_pRightNeighbor->m_nInstanceId;
				}
				break;
			case NeighborType::neighborPositionTop:
				if (m_pTopNeighbor)
				{
					return m_pTopNeighbor->m_nInstanceId;
				}
				break;
			default:
				break;
		}
		return 0;
	}
	void Terrain::Init(float* heightMap, int32_t heightMapSize)
	{
		if (!heightMap)
		{
			LogError("TerrianDataBinding and heightMap cant be null");
			return;
		}
		//LogFormat("init terrain %d data",m_nInstanceId);
		m_bHeightMapInitilized.store(true, std::memory_order_relaxed);
		m_aHeightMap = heightMap;
		m_nSize = heightMapSize;
		if (m_nRealWidth == 0)
		{
			m_nRealWidth = m_nSize;
		}
		//LogFormat("terrain %d real size %d", m_nInstanceId, m_nRealWidth);
	}

	void Terrain::InitNeighbor(NeighborType edge, std::shared_ptr<Terrain> neighbor)
	{
		//LogFormat("terrain %d InitNeighbor %d,neighbor %d", m_nInstanceId, edge, neighbor->m_nInstanceId);
		switch (edge)
		{
			case NeighborType::neighborPositionLeft:
				m_pLeftNeighbor = neighbor;
				if (neighbor && neighbor->m_pRightNeighbor != nullptr && neighbor->m_pRightNeighbor != shared_from_this())
				{
					LogErrorFormat("fail to set neighbor ,left neighbor's right neighbor is not self");
					m_pLeftNeighbor = nullptr;
					return;
				}
				break;
			case  NeighborType::neighborPositionRight:
				m_pRightNeighbor = neighbor;
				if (neighbor && neighbor->m_pLeftNeighbor != nullptr && neighbor->m_pLeftNeighbor != shared_from_this())
				{
					LogErrorFormat("fail to set neighbor ,right neighbor's left neighbor is not self");
					m_pRightNeighbor = nullptr;
					return;
				}
				break;
			case NeighborType::neighborPositionBottom:
				m_pBottomNeighbor = neighbor;
				if (neighbor && neighbor->m_pTopNeighbor != nullptr && neighbor->m_pTopNeighbor != shared_from_this())
				{
					LogErrorFormat("fail to set neighbor ,bottom neighbor's top neighbor is not self");
					m_pBottomNeighbor = nullptr;
					return;
				}
				break;
			case  NeighborType::neighborPositionTop:
				m_pTopNeighbor = neighbor;
				if (neighbor && neighbor->m_pBottomNeighbor != nullptr && neighbor->m_pBottomNeighbor != shared_from_this())
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
	bool Terrain::GetNeighborHeight(int32_t x, int32_t y, NeighborType neighbor, float & p)
	{
		switch (neighbor)
		{
			case NeighborType::neighborPositionLeft:
				if (m_pLeftNeighbor)
				{
					p = m_pLeftNeighbor->GetHeight(x, y);
					LogFormat("terrain %d GetNeighborHeight neighborPositionLeft  %f", m_nInstanceId, p);
					return true;
				}
				break;
			case  NeighborType::neighborPositionRight:
				if (m_pRightNeighbor)
				{
					p = m_pRightNeighbor->GetHeight(x, y);
					LogFormat("terrain %d GetNeighborHeight neighborPositionRight  %f", m_nInstanceId, p);
					return true;
				}
				break;
			case NeighborType::neighborPositionBottom:
				if (m_pBottomNeighbor)
				{
					p = m_pBottomNeighbor->GetHeight(x, y);
					LogFormat("terrain %d GetNeighborHeight neighborPositionBottom  %f", m_nInstanceId, p);
					return true;
				}
				break;
			case  NeighborType::neighborPositionTop:
				if (m_pTopNeighbor)
				{
					p = m_pTopNeighbor->GetHeight(x, y);
					LogFormat("terrain %d GetNeighborHeight neighborPositionTop  %f", m_nInstanceId, p);
					return true;
				}
				break;
			default:
				break;
		}
		return false;
	}
};