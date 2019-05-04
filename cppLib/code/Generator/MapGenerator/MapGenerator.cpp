#include "MapGenerator.h"
#include "MapGenerator.h"
#include "Logger/leakHelper.h"
#include "Logger/Logger.h"
#include "G3D/Vector4.h"
using namespace logger;
namespace generator
{
	static MapGenerator* instance = nullptr;
	MapGenerator::MapGenerator() :
		m_cbTerrainGenFinish(nullptr),
		m_aHeightMap(nullptr),
		m_aSplatMap(nullptr),
		m_nCurrentTerrain(0),
		m_pGenerator(nullptr),
		m_pPainter(nullptr),
		m_pWorldMap(nullptr),
		m_pThreadRunner(nullptr),
		m_bRun(false)
	{
		m_mTerrainData.clear();
		m_pGenerator = new Diamond_Square();
		m_pPainter = new AutomaticPainter();
	}

	MapGenerator::~MapGenerator()
	{
		Stop();
		safe_delete(m_pGenerator);
		safe_delete(m_pPainter);
		safe_delete(m_pWorldMap);
	}

	void MapGenerator::Init(MapGeneratorData data)
	{
		m_pGenerator->Initilize(0, data.seed, data.I, data.H, m_aHeightMap);
		data.singleMapSize = m_pGenerator->GetSquareSize();
		m_stData = data;
		m_nTotalMapCount = data.worldMapSize * data.worldMapSize;
		m_worldMapHeightMap = new float[data.singleMapSize*data.singleMapSize / 10000];//world map is 100 sizes smaller than single map size
		m_aHeightMap = new float[data.singleMapSize*data.singleMapSize];
		m_aSplatMap = new float[data.splatWidth*data.splatWidth*data.splatCount];
	}

	MapGenerator * MapGenerator::GetInstance()
	{
		if (!instance)
		{
			instance = new	MapGenerator();
		}
		return instance;
	}

	void MapGenerator::Destroy()
	{
		safe_delete(instance);
	}

	void MapGenerator::StartRun()
	{
		if (!m_pThreadRunner)
		{
			m_pThreadRunner = new std::thread(std::bind(&MapGenerator::WorkThread, this));
			m_pThreadRunner->detach();
		}
	}

	void MapGenerator::Stop()
	{
		if (m_pThreadRunner)
		{
			m_bRun = false;
			m_pThreadRunner->join();
			safe_delete(m_pThreadRunner);
		}
	}

	void MapGenerator::UpdateInMainThread(int32_t diff)
	{
		uint32_t next;
		while (m_finishQueue.next(next))
		{
			LogFormat("UpdateInMainThread next %d cb %d", next, m_cbTerrainGenFinish);
			if (m_cbTerrainGenFinish)
			{
				m_cbTerrainGenFinish(next, m_stData.singleMapSize, G3D::Vector4(m_pCurrentMap->m_Position.x, m_pCurrentMap->m_Position.y, m_pCurrentMap->m_Position.z, m_pCurrentMap->m_nRealWidth));
			}
		}
	}
	void MapGenerator::InitTerrainInMainThread(uint32_t terrain, float* heightMap, int32_t heightMapSize, float* splatMap, int32_t splatWidth, int32_t splatCount)
	{
		if (!heightMap)
		{
			return;
		}
		if (!splatMap)
		{
			return;
		}
		if (heightMapSize != m_stData.singleMapSize)
		{
			return;
		}
		if (splatWidth != m_stData.splatWidth)
		{
			return;
		}
		if (splatCount != m_stData.splatCount)
		{
			return;
		}
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		auto itr = m_mTerrainData.find(terrain);
		if (itr != m_mTerrainData.end())
		{
			itr->second->Init(m_stData.I, heightMap, heightMapSize, splatMap, splatWidth, splatCount);
			LogFormat("Init terrain 0");
			memcpy(heightMap, m_aHeightMap, sizeof(m_aHeightMap));
			memcpy(splatMap, m_aSplatMap, sizeof(m_aSplatMap));
			LogFormat("Init terrain 1");
		}
	}
	std::shared_ptr<Terrain> MapGenerator::GetTerrain(uint32_t terr)
	{
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		auto itr = m_mTerrainData.find(terr);
		if (itr != m_mTerrainData.end())
		{
			return itr->second;
		}
		return nullptr;
	}
	void MapGenerator::SaveTerrain(uint32_t terr)
	{
	}
	void MapGenerator::WorkThreadEntry()
	{
		//if (!m_bRun)
		{
			WorkThread();
		}
	}
	void MapGenerator::WorkThread()
	{
		m_bRun = true;
		GenWorldMap();
		Generate(0xffffffff);
		while (m_bRun)
		{
			//sleep(10);
			uint32_t current = InitilizeNext();
			if (current > 0)
			{
				Generate(current);
			}
			else if (current > 2)
			{
				m_bRun = false;
			}
			UpdateInMainThread(0);
			while (!m_finishQueue.empty())
			{
				sleep(100);//hold on while finish queue is empty
			}
		}
	}
	uint32 MapGenerator::InitilizeNext()
	{
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		return (uint32)m_mTerrainData.size() + 1;
	}
	void MapGenerator::Generate(uint32 terr)
	{
		m_nCurrentTerrain = terr;
		LogFormat("Generate terrain %d start", terr);
		if (terr != 0xffffffff)//not world map
		{
			m_pCurrentMap = std::make_shared<Terrain>(terr, 0);
			uint32_t neighbor[4] = { 0 };
			for (size_t i = 0; i < 4; i++)
			{
				auto dir = (NeighborType)i;
				neighbor[i] = GetNeighborID(dir, terr);
				std::shared_ptr<Terrain> s_ptr = nullptr;
				{
					std::lock_guard<std::mutex> lock(m_generatorMtx);
					s_ptr = m_mTerrainData[neighbor[i]];
				}
				if (s_ptr != nullptr)
				{
					m_pCurrentMap->InitNeighbor(dir, s_ptr);
					InitHeightMapBaseOnNeighbor(dir);
				}
			}
		}
		LogFormat("Generate terrain %d 1", terr);
		if (!LoadFromNative(terr))
		{
			GenerateTerrain();
			LogFormat("Generate terrain %d 2", terr);
			if (!m_pCurrentMap->IsWorldMap())//not world map
			{
				//todo generate
				AutoGenSplatMap();
				std::lock_guard<std::mutex> lock(m_generatorMtx);
				m_mTerrainData.insert(std::make_pair(terr, m_pCurrentMap));
				m_finishQueue.add(terr);
			}
		}
		if (m_pCurrentMap->IsWorldMap())
		{
			memcpy(m_worldMapHeightMap, m_aHeightMap, sizeof(m_aHeightMap));
		}
		LogFormat("gen terrain %d finish 1", m_pCurrentMap->m_nInstanceId);
	}

	uint32_t MapGenerator::GetNeighborID(NeighborType dir, uint32_t who)
	{
		if (who == 0)
		{
			return 0;
		}
		uint32_t worldMapSize = m_stData.worldMapSize;
		uint32_t x = who % m_stData.worldMapSize;
		uint32_t y = who / m_stData.worldMapSize;
		int32_t idx = 0;
		switch (dir)
		{
			case NeighborType::neighborPositionLeft:
				idx = x - 1 + y * worldMapSize;
				return idx > 0 ? idx : 0;
			case NeighborType::neighborPositionBottom:
				idx = x + (y - 1) * worldMapSize;
				return idx > 0 ? idx : 0;
			case NeighborType::neighborPositionRight:
				idx = x + 1 + y * worldMapSize;
				return idx <= m_nTotalMapCount ? idx : 0;
			case NeighborType::neighborPositionTop:
				idx = x + (y + 1) * worldMapSize;
				return idx <= m_nTotalMapCount ? idx : 0;
			default:
				return 0;
				break;
		}
	}
	bool MapGenerator::LoadFromNative(uint32_t terr)
	{
		//todo load raw map
		return false;
	}
	void MapGenerator::GenWorldMap()
	{
		m_pWorldMap = new Terrain(0xffffffff, 0);
		m_pWorldMap->Init(m_stData.I, m_worldMapHeightMap, m_stData.singleMapSize / 100, nullptr, 0, 0);
		m_pCurrentMap = std::make_shared<Terrain>(*m_pWorldMap);
	}
	void MapGenerator::AutoGenSplatMap()
	{
		m_pPainter->Init(m_aHeightMap, m_stData.singleMapSize, m_aSplatMap, m_stData.splatWidth, m_stData.splatCount);
		m_pPainter->DrawSplatMap();
	}

	void MapGenerator::GenerateTerrain()
	{
		LogFormat("Generate terrain %d 3", m_pCurrentMap->m_nInstanceId);
		m_pGenerator->Initilize(m_pCurrentMap->m_nInstanceId, std::rand(), m_stData.I, m_stData.H, m_aHeightMap);
		m_pCurrentMap->m_nSize = m_pGenerator->GetSquareSize();
		static float cornor[] = { m_stData.height0,m_stData.height1,m_stData.height2,m_stData.height3 };
		m_pGenerator->Start(cornor, 4, m_stData.singleMapSize);
		LogFormat("Generate terrain %d 4", m_pCurrentMap->m_nInstanceId);
	}

	bool MapGenerator::GetNeighborHeight(int32_t x, int32_t y, NeighborType neighbor, uint32_t neighborID, float & p)
	{
		//when terrain is initilizing in main thread,subthread eneration is not running,so no need to add mutex lock here
		const std::map<uint32_t, std::shared_ptr<Terrain>>::iterator& itr = sMapGenerator->m_mTerrainData.find(neighborID);
		if (itr != sMapGenerator->m_mTerrainData.end())
		{
			const std::shared_ptr<Terrain>& terrain = itr->second;
			if (terrain)
			{
				return terrain->GetNeighborHeight(x, y, neighbor, p);
			}
		}
		return false;
	}
	void MapGenerator::InitHeightMapBaseOnNeighbor(NeighborType position)
	{
		int m_nSize = m_stData.singleMapSize;
		int nMax = m_nSize - 1;
		LogFormat("InitHeightMapBaseOnNeighbor pos %d,for terrain %d", position, m_pCurrentMap->m_nInstanceId);
		if (position == NeighborType::neighborPositionLeft)
		{
			if (m_pCurrentMap->m_pLeftNeighbor)
			{
				//LogFormat("init left pulse ");
				if (m_pCurrentMap->m_pLeftNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pCurrentMap->m_pLeftNeighbor->m_nSize);
					return;
				}
				else if (m_pCurrentMap->m_pLeftNeighbor->m_nSize > 0)
				{
					//add left edge
					int x = 0;
					int nx(x + nMax);
					LogFormat("t %d m_pLeftNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pLeftNeighbor->m_nInstanceId, m_pCurrentMap->m_pLeftNeighbor->m_aHeightMap);
					for (int y = 0; y <= nMax; y++)
					{
						float nheight = m_pCurrentMap->m_pLeftNeighbor->GetHeight(nx, y);
						m_pGenerator->SetPulse(x, y, nheight);
						//LogFormat("SetPulse x %d,y %d, nx %d,height %f", x, y, nx, nheight);
					}
				}
			}
		}

		if (position == NeighborType::neighborPositionRight)
		{
			if (m_pCurrentMap->m_pRightNeighbor)
			{
				//add right edge
				if (m_pCurrentMap->m_pRightNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pCurrentMap->m_pRightNeighbor->m_nSize);
					return;
				}
				else if (m_pCurrentMap->m_pRightNeighbor->m_nSize > 0)
				{
					int x = nMax;
					int nx(x - nMax);
					LogFormat("t %d m_pRightNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pRightNeighbor->m_nInstanceId, m_pCurrentMap->m_pRightNeighbor->m_aHeightMap);
					for (int y = 0; y <= nMax; y++)
					{
						float nheight = m_pCurrentMap->m_pRightNeighbor->GetHeight(nx, y);
						m_pGenerator->SetPulse(x, y, nheight);
					}
				}
			}
		}

		if (position == NeighborType::neighborPositionBottom)
		{
			if (m_pCurrentMap->m_pBottomNeighbor)
			{
				//add bottom edge
				if (m_pCurrentMap->m_pBottomNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pCurrentMap->m_pBottomNeighbor->m_nSize);
					return;
				}
				else if (m_pCurrentMap->m_pBottomNeighbor->m_nSize > 0)
				{
					int y = 0;
					int nY(y + nMax);
					LogFormat("t %d m_pBottomNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pBottomNeighbor->m_nInstanceId, m_pCurrentMap->m_pBottomNeighbor->m_aHeightMap);
					for (int x = 0; x <= nMax; x++)
					{
						float nheight = m_pCurrentMap->m_pBottomNeighbor->GetHeight(x, nY);
						m_pGenerator->SetPulse(x, y, nheight);
					}
				}
			}
		}

		if (position == NeighborType::neighborPositionTop)
		{
			if (m_pCurrentMap->m_pTopNeighbor)
			{
				//add top edge
				if (m_pCurrentMap->m_pTopNeighbor->m_nSize != m_nSize)
				{
					LogErrorFormat("neighbor size %d is not equal to neighbor %d", m_nSize, m_pCurrentMap->m_pTopNeighbor->m_nSize);
					return;
				}
				else if (m_pCurrentMap->m_pTopNeighbor->m_nSize > 0)
				{
					int y = nMax;
					int nY(y - nMax);
					LogFormat("t %d m_pTopNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pTopNeighbor->m_nInstanceId, m_pCurrentMap->m_pTopNeighbor->m_aHeightMap);
					for (int x = 0; x <= nMax; x++)
					{
						float nheight = m_pCurrentMap->m_pTopNeighbor->GetHeight(x, nY);
						m_pGenerator->SetPulse(x, y, nheight);
					}
				}
			}
		}
	}
	static uint32_t mapId_instance = 0;
	uint32_t MapInstanceIdGenerator::GetInstanceId()
	{
		return mapId_instance++;
	}

}