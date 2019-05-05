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
		m_bRun(false),
		m_bThreadExited(true)
	{
		m_mTerrainData.clear();
		m_pGenerator = new Diamond_Square();
		m_pGenerator->SetGetNeighborHeightCallBack(MapGenerator::GetNeighborHeight);
		m_pPainter = new AutomaticPainter();
	}

	MapGenerator::~MapGenerator()
	{
		Stop();
		safe_delete(m_pGenerator);
		safe_delete(m_pPainter);
		LogFormat("1");
		safe_delete_array(m_aSplatMap);
		safe_delete_array(m_aSplatMapCopy);
		safe_delete_array(m_aHeightMap);
		safe_delete_array(m_aHeightMapCopy);
		LogFormat("2");
		m_pWorldMap = nullptr;
		LogFormat("3");
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
		m_aHeightMapCopy = new float[data.singleMapSize*data.singleMapSize];
		m_aSplatMapCopy = new float[data.splatWidth*data.splatWidth*data.splatCount];
		LogFormat("MapGenerator::Init mapwidth %d,total map count %d,worldmapSize %d", m_stData.singleMapSize, m_nTotalMapCount, m_stData.worldMapSize);
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
			while (!m_bThreadExited)
			{
				sleep(1);
			}
			//m_pThreadRunner->join();
			safe_delete(m_pThreadRunner);
		}
		LogFormat("MapGenerator WorkThread exited");
	}

	void MapGenerator::UpdateInMainThread(int32_t diff)
	{
		if (m_mTerrainData.size() >= 4 && m_finishQueue.empty())
		{
			LogFormat("MapGenerator::UpdateInMainThread(int32_t diff) start");
		}
		uint32_t next;
		while (m_finishQueue.next(next))
		{
			LogFormat("UpdateInMainThread next %d empty %d", next, m_finishQueue.empty());
			if (m_cbTerrainGenFinish)
			{
				m_cbTerrainGenFinish(next, m_stData.singleMapSize, G3D::Vector4(m_pCurrentMap->m_Position.x, m_pCurrentMap->m_Position.y, m_pCurrentMap->m_Position.z, m_pCurrentMap->m_nRealWidth));
			}
		}
		if (m_mTerrainData.size() >= 4 && m_finishQueue.empty())
		{
			LogFormat("MapGenerator::UpdateInMainThread(int32_t diff) end");
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
		LogFormat("InitTerrainInMainThread %d,%d,%d", m_pCurrentMap->m_nInstanceId, heightMap, splatMap);
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		auto itr = m_mTerrainData.find(terrain);
		if (itr != m_mTerrainData.end())
		{
			if (itr->second->m_nInstanceId != terrain)
			{
				LogErrorFormat("fail to init terrain %d", terrain);
				return;
			}
			itr->second->Init(heightMap, heightMapSize, splatMap, splatWidth, splatCount);
			//memcpy(heightMap, m_aHeightMap, sizeof(m_aHeightMap));
			//memcpy(splatMap, m_aSplatMap, sizeof(m_aSplatMap));
			int idx = 0;
			for (int y = 0; y < m_stData.singleMapSize; y++)
			{
				for (int x = 0; x < m_stData.singleMapSize; x++)
				{
					heightMap[idx] = m_aHeightMapCopy[idx];
					idx++;
				}
			}
			idx = 0;
			for (int y = 0; y < m_stData.splatWidth; y++)
			{
				for (int x = 0; x < m_stData.splatWidth; x++)
				{
					for (int z = 0; z < m_stData.splatCount; z++)
					{
						splatMap[idx] = m_aSplatMapCopy[idx];
						idx++;
					}
				}
			}
		}
		else
		{

			LogFormat("Init terrain fail ,terrain  %d not found", terrain);
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
		/*{
			WorkThread();
		}*/
	}
	void MapGenerator::WorkThread()
	{
		m_bRun = true;
		GenWorldMap();
		m_bThreadExited = false;
		while (m_bRun)
		{
			//sleep(10);
			uint32_t current = InitilizeNext();
			LogFormat("current gen terrain %d ,m_nTotalMapCount %d", current, m_nTotalMapCount);
			if (current > 0 && current <= m_nTotalMapCount)
			{
				Generate(current);
			}
			//UpdateInMainThread(0);
			while (!m_finishQueue.empty())
			{
				sleep(100);//hold on while finish queue is empty
				LogFormat("finish queue empty %d", m_finishQueue.empty());
			}
			if (current >= m_nTotalMapCount)
			{
				break;;
			}
		}
		m_bRun = false;
		m_bThreadExited = true;
		LogFormat("Gen WorkThread Exited");
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
		m_pCurrentMap = std::make_shared<Terrain>(terr, m_stData.I);
		uint32_t neighbor[4] = { 0 };
		for (size_t i = 0; i < 4; i++)
		{
			auto dir = (NeighborType)i;
			neighbor[i] = GetNeighborID(dir, terr);
			std::shared_ptr<Terrain> s_ptr = nullptr;
			{
				std::lock_guard<std::mutex> lock(m_generatorMtx);
				auto itr = m_mTerrainData.find(neighbor[i]);
				if (itr != m_mTerrainData.end())
				{
					s_ptr = m_mTerrainData[neighbor[i]];
				}
			}
			if (s_ptr != nullptr)
			{
				m_pCurrentMap->InitNeighbor(dir, s_ptr);
				InitHeightMapBaseOnNeighbor(dir, s_ptr);
			}
		}
		if (!LoadFromNative(terr))
		{
			uint32_t index = terr - 1;
			uint32_t x = index % m_stData.worldMapSize;
			uint32_t y = index / m_stData.worldMapSize;
			float fx = x * m_pCurrentMap->m_nRealWidth;
			float fy = y * m_pCurrentMap->m_nRealWidth;
			m_pCurrentMap->m_Position = G3D::Vector3(fx, 0, fy);
			LogFormat("map %d size x %f,size z %f, x %,y %d", terr, fx, fy, x, y);
			GenerateTerrain();
			AutoGenSplatMap();
			std::lock_guard<std::mutex> lock(m_generatorMtx);
			m_mTerrainData.insert(std::make_pair(terr, m_pCurrentMap));

			int idx = 0;
			for (int y = 0; y < m_stData.singleMapSize; y++)
			{
				for (int x = 0; x < m_stData.singleMapSize; x++)
				{
					m_aHeightMapCopy[idx] = m_aHeightMap[idx];
					idx++;
				}
			}
			idx = 0;
			for (int y = 0; y < m_stData.splatWidth; y++)
			{
				for (int x = 0; x < m_stData.splatWidth; x++)
				{
					for (int z = 0; z < m_stData.splatCount; z++)
					{
						m_aSplatMapCopy[idx] = m_aSplatMap[idx];
						idx++;
					}
				}
			}
			m_finishQueue.add(terr);
			LogFormat("gen terrain %d finish ,current terrain count %d ", m_pCurrentMap->m_nInstanceId, m_mTerrainData.size());
		}
	}
	void MapGenerator::GenWorldMap()
	{
		m_pWorldMap = std::make_shared<Terrain>(0xffffffff, m_stData.I / 2);
		m_pWorldMap->Init(m_worldMapHeightMap, m_pGenerator->GetSquareSize(), nullptr, 0, 0);
		if (!LoadFromNative(0xffffffff))
		{
			m_pGenerator->Initilize(m_pWorldMap->m_nInstanceId, std::rand(), m_pWorldMap->GetI(), m_stData.H, m_worldMapHeightMap);
			float worldMapConor[] = { m_stData.height0,m_stData.height1,m_stData.height2,m_stData.height3 };
			m_pGenerator->Start(worldMapConor, 4, 0);
		}
	}
	void MapGenerator::AutoGenSplatMap()
	{
		LogFormat("Generate terrain %d splat map start", m_pCurrentMap->m_nInstanceId);
		m_pPainter->Init(m_aHeightMap, m_stData.singleMapSize, m_aSplatMap, m_stData.splatWidth, m_stData.splatCount);
		m_pPainter->DrawSplatMap();
		LogFormat("Generate terrain %d splat map finished", m_pCurrentMap->m_nInstanceId);
	}

	void MapGenerator::GenerateTerrain()
	{
		LogFormat("Generate terrain %d height map start", m_pCurrentMap->m_nInstanceId);
		m_pGenerator->Initilize(m_pCurrentMap->m_nInstanceId, std::rand(), m_stData.I, m_stData.H, m_aHeightMap);
		static float cornor[] = { m_stData.height0,m_stData.height1,m_stData.height2,m_stData.height3 };
		m_pGenerator->Start(cornor, 4, 0);
		LogFormat("Generate terrain %d height map finished", m_pCurrentMap->m_nInstanceId);
	}
	uint32_t MapGenerator::GetNeighborID(NeighborType dir, uint32_t who)
	{
		if (who == 0)
		{
			return 0;
		}
		uint32_t worldMapSize = m_stData.worldMapSize;
		uint32_t index = who - 1;
		uint32_t x = index % m_stData.worldMapSize;
		uint32_t y = index / m_stData.worldMapSize;
		int32_t idx = 0;
		switch (dir)
		{
			case NeighborType::neighborPositionLeft:
				idx = x - 1 + y * worldMapSize + 1;
				return idx > 0 ? idx : 0;
			case NeighborType::neighborPositionBottom:
				idx = x + (y - 1) * worldMapSize + 1;
				return idx > 0 ? idx : 0;
			case NeighborType::neighborPositionRight:
				idx = x + 1 + y * worldMapSize + 1;
				return idx <= m_nTotalMapCount ? idx : 0;
			case NeighborType::neighborPositionTop:
				idx = x + (y + 1) * worldMapSize + 1;
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

	bool MapGenerator::GetNeighborHeight(int32_t x, int32_t y, NeighborType neighbor, uint32_t owner, float & p)
	{
		//when terrain is initilizing in main thread,subthread eneration is not running,so no need to add mutex lock here
		const std::map<uint32_t, std::shared_ptr<Terrain>>::iterator& itr = sMapGenerator->m_mTerrainData.find(owner);
		if (itr != sMapGenerator->m_mTerrainData.end())
		{
			const std::shared_ptr<Terrain>& terrain = itr->second;
			if (terrain)
			{
				bool ret = terrain->GetNeighborHeight(x, y, neighbor, p);
				LogFormat("owner %d GetNeighborHeight ret %d,height %f", owner, ret, p);
				return ret;
			}
		}
		return false;
	}
	void MapGenerator::InitHeightMapBaseOnNeighbor(NeighborType position, std::shared_ptr<Terrain> neighbor)
	{
		int m_nSize = m_stData.singleMapSize;
		int nMax = m_nSize - 1;
		LogFormat("InitHeightMapBaseOnNeighbor pos %d,for terrain %d", position, m_pCurrentMap->m_nInstanceId);
		if (position == NeighborType::neighborPositionLeft)
		{
			if (m_pCurrentMap->m_pLeftNeighbor == neighbor)
			{
				//add left edge
				int x = 0;
				int nx(x + nMax);
				LogFormat("t %d m_pLeftNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pLeftNeighbor->m_nInstanceId, m_pCurrentMap->m_pLeftNeighbor->m_aHeightMap);
				for (int y = 0; y <= nMax; y++)
				{
					float nheight = m_pCurrentMap->m_pLeftNeighbor->GetHeight(nx, y);
					m_pGenerator->SetPulse(x, y, nheight);
					//LogFormat("%d SetPulse x %d,y %d, nx %d,height %f", m_pCurrentMap->m_nInstanceId, x, y, nx, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionRight)
		{
			if (m_pCurrentMap->m_pRightNeighbor == neighbor)
			{
				int x = nMax;
				int nx(x - nMax);
				LogFormat("t %d m_pRightNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pRightNeighbor->m_nInstanceId, m_pCurrentMap->m_pRightNeighbor->m_aHeightMap);
				for (int y = 0; y <= nMax; y++)
				{
					float nheight = m_pCurrentMap->m_pRightNeighbor->GetHeight(nx, y);
					m_pGenerator->SetPulse(x, y, nheight);
					//LogFormat("%d SetPulse x %d,y %d, nx %d,height %f", m_pCurrentMap->m_nInstanceId, x, y, nx, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionBottom)
		{
			if (m_pCurrentMap->m_pBottomNeighbor == neighbor)
			{
				int y = 0;
				int nY(y + nMax);
				LogFormat("t %d m_pBottomNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pBottomNeighbor->m_nInstanceId, m_pCurrentMap->m_pBottomNeighbor->m_aHeightMap);
				for (int x = 0; x <= nMax; x++)
				{
					float nheight = m_pCurrentMap->m_pBottomNeighbor->GetHeight(x, nY);
					m_pGenerator->SetPulse(x, y, nheight);
					//LogFormat("%d SetPulse x %d,y %d, nx %d,height %f", m_pCurrentMap->m_nInstanceId, x, y, nY, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionTop)
		{
			if (m_pCurrentMap->m_pTopNeighbor == neighbor)
			{
				int y = nMax;
				int nY(y - nMax);
				LogFormat("t %d m_pTopNeighbor neighbor %d heightmap %d", m_pCurrentMap->m_nInstanceId, m_pCurrentMap->m_pTopNeighbor->m_nInstanceId, m_pCurrentMap->m_pTopNeighbor->m_aHeightMap);
				for (int x = 0; x <= nMax; x++)
				{
					float nheight = m_pCurrentMap->m_pTopNeighbor->GetHeight(x, nY);
					m_pGenerator->SetPulse(x, y, nheight);
					//LogFormat("%d SetPulse x %d,y %d, nx %d,height %f", m_pCurrentMap->m_nInstanceId, x, y, nY, nheight);
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