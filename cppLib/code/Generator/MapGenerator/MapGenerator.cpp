#include "MapGenerator.h"
#include "Logger/leakHelper.h"
#include "Logger/Logger.h"
#include "G3D/Vector4.h"
#include "Threading/ThreadManager.h"
using namespace logger;
namespace generator
{
	/*
	 *Note
	 *说明
	 *地图生成器
	 *当前版本是生成一个worldMap，然后再生成小块地图（I<6，I>=6时地图太大，时间消耗太久）
	 *小块地图在生成时，在I/2的生成循环内是获取世界地图的高度，之后是通过diamond-square规则生成
	 *
	 *目前有个效果比较好（地图拼接，加载快）的方案如下
	 *worldMap设置成i>=6的地图，生成结束后，把worldMap切块成64*64(如果I=6，则要切成4096/64的平方个地图快)
	 *这样在实际显示的时候，地图边缘的细节会暴露得比较少，同时因为切块了，地图动态加载时卡顿也会大幅度减少
	 *
	 *但是目前计划的重点不是优化地图，这个可以在后面返回来做
	 */
#define break_if_stopped if(!m_bRun) break
#define return_if_stopped if(!m_bRun) return
	static std::shared_ptr<MapGenerator>  instance = nullptr;
	MapGenerator::MapGenerator() :
		m_pWorldMap(nullptr),
		m_pGenerator(nullptr),
		m_pPainter(nullptr),
		m_worldMapHeightMap(nullptr),
		//m_aHeightMap(nullptr),
		m_aSplatMap(nullptr),
		m_aHeightMapCopy(nullptr),
		m_aSplatMapCopy(nullptr),
		m_cbTerrainGenFinish(nullptr),
		m_nCurrentTerrain(0),
		m_stData(),
		m_bRun(false),
		m_bThreadExited(true),
		m_nTotalMapCount(0)
	{
		m_mTerrainData.clear();
		m_pGenerator = new Diamond_Square();
		m_pGenerator->SetGetNeighborHeightCallBack(MapGenerator::GetHeightOnNeighbor);
		m_pGenerator->SetGetWorldHeightCallBack(MapGenerator::GetHeightOnWorldMap);
		m_pPainter = new AutomaticPainter();
	}

	MapGenerator::~MapGenerator()
	{
		Stop();
		safe_delete(m_pGenerator);
		safe_delete(m_pPainter);
		//safe_delete_array(m_aHeightMap);
		safe_delete_array(m_aHeightMapCopy);
		safe_delete_array(m_aSplatMap);
		safe_delete_array(m_aSplatMapCopy);
		m_pWorldMap = nullptr;
		LogFormat("map generator exit");
	}

	void MapGenerator::Init(MapGeneratorData data)
	{
		//m_pGenerator->Initilize(0, data.seed, data.I, 0, nullptr);
		//data.singleMapSize = m_pGenerator->GetSquareSize();
		m_stData = data;
		m_pGenerator->Initilize(0, 0, GetWorldMapSize(), 0, nullptr);
		uint32_t worldMapSize = m_pGenerator->GetSquareSize();
		const int singleSize = worldMapSize / (data.singleMapSize - 1);
		m_nTotalMapCount = /*data.worldMapSize*/1 * singleSize*singleSize;//data.worldMapSize * data.worldMapSize;
		m_worldMapHeightMap = new float[worldMapSize*worldMapSize];
		const uint32_t splatSize = worldMapSize * data.splatWidth / (float)data.singleMapSize;
		m_aSplatMap = new float[splatSize*splatSize*data.splatCount];
		//m_aHeightMap = new float[data.singleMapSize*data.singleMapSize];
		m_aHeightMapCopy = new float[data.singleMapSize*data.singleMapSize];
		m_aSplatMapCopy = new float[data.splatWidth*data.splatWidth*data.splatCount];
		m_pPainter->Init(m_worldMapHeightMap, worldMapSize, m_aSplatMap, splatSize, m_stData.splatCount);
		LogFormat("MapGeneratorData seed %d,i %d,h %d,worldSize %d,single size %d,map count per edge %d,splat size %d,splat count %d,m_nTotalMapCount %d", data.seed, data.I, data.H, worldMapSize, data.singleMapSize, singleSize, data.splatWidth, data.splatCount, m_nTotalMapCount);
	}

	std::shared_ptr<MapGenerator>  MapGenerator::GetInstance()
	{
		if (!instance)
		{
			instance = std::make_shared<MapGenerator>();
		}
		return instance;
	}

	void MapGenerator::Destroy()
	{
		instance = nullptr;
	}

	void MapGenerator::StartRun()
	{
		m_bRun = true;
		LogFormat("MapGenerator start run");
		threading::ThreadManager::GetInstance()->AddTask(threading::ThreadTask(std::bind(&MapGenerator::WorkThread, instance)));
	}

	void MapGenerator::Stop()
	{
		m_bRun = false;
		LogFormat("MapGenerator WorkThread exited");
	}

	void MapGenerator::UpdateInMainThread(int32_t diff)
	{
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		uint32_t next;
		while (m_finishQueue.next(next))
		{
			if (m_cbTerrainGenFinish)
			{
				m_cbTerrainGenFinish(next, m_stData.singleMapSize, G3D::Vector4(m_pCurrentMap->m_Position.x, m_pCurrentMap->m_Position.y, m_pCurrentMap->m_Position.z, m_pCurrentMap->GetRealSize()));
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
		LogFormat("InitTerrainInMainThread %d,cur %d,%d,%d", terrain, m_pCurrentMap->m_nInstanceId, heightMap, splatMap);
		//std::lock_guard<std::mutex> lock(m_generatorMtx);
		auto itr = m_mTerrainData.find(terrain);
		if (itr != m_mTerrainData.end())
		{
			if (itr->second->m_nInstanceId != terrain)
			{
				LogErrorFormat("fail to init terrain %d", terrain);
				return;
			}
			itr->second->Init(heightMap, heightMapSize);
			memcpy(heightMap, m_aHeightMapCopy, m_stData.singleMapSize*m_stData.singleMapSize * sizeof(float));
			memcpy(splatMap, m_aSplatMapCopy, m_stData.splatWidth*m_stData.splatWidth * m_stData.splatCount * sizeof(float));
		}
		else
		{

			LogFormat("Init terrain fail ,terrain  %d not found", terrain);
		}
	}
	std::shared_ptr<Terrain> MapGenerator::GetTerrain(uint32_t terr)
	{
		//std::lock_guard<std::mutex> lock(m_generatorMtx);
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
		LogError("WorkThreadEntry is not avalible any more");
		/*if (!m_bRun && m_stData.flags & 0x3)
		{
			WorkThread();
		}*/
	}
	bool MapGenerator::HasInstance()
	{
		return instance != nullptr;
	}
	void MapGenerator::WorkThread()
	{
		LogFormat("WorkThread start ");
		if (m_bRun)
			GenWorldMap();
		m_bThreadExited = false;
		while (m_bRun)
		{
			//sleep(10);
			uint32_t current = InitilizeNext();
			LogFormat("current gen terrain %d ,m_nTotalMapCount %d", current, m_nTotalMapCount);
			if (current > 0)
			{
				Generate(current);
			}
			while (!m_finishQueue.empty())
			{
				break_if_stopped;
				sleep(2);//hold on while finish queue is empty
			}
			break_if_stopped;
			if (current >= m_nTotalMapCount)
			{
				break;
			}
		}
		m_bRun = false;
		m_bThreadExited = true;
		LogFormat("Gen WorkThread Exited");
	}
	uint32 MapGenerator::InitilizeNext()
	{
		std::lock_guard<std::mutex> lock(m_generatorMtx);
		return static_cast<uint32>(m_mTerrainData.size()) + 1;
	}
	void MapGenerator::Generate(uint32 terr)
	{
		m_nCurrentTerrain = terr;
		//LogFormat("Generate terrain %d start", terr);
		m_pCurrentMap = std::make_shared<Terrain>(terr, 1, m_stData.singleMapSize);
		const uint32_t mapIndex = terr - 1;
		const uint32_t m_nSize = m_pWorldMap->GetHeightMapSize();
		const uint32_t mapCountPerEdge = m_nSize / (m_stData.singleMapSize - 1);
		//calculate map index x,y (x+y*mapCountPerEdge = totalMapCount)
		const uint32_t x_index = mapIndex % mapCountPerEdge;
		const uint32_t y_index = mapIndex / mapCountPerEdge;
		const float fx = x_index * m_pCurrentMap->GetRealSize();
		const float fy = y_index * m_pCurrentMap->GetRealSize();
		m_pCurrentMap->m_Position = G3D::Vector3(fx, 0, fy);
		LogFormat("map %d size x %f,size z %f, x %,y %d,mapCountPerEdge %d,m_nSize %d ", terr, fx, fy, x_index, y_index, mapCountPerEdge, m_nSize);
		//initilize points based on world map
		return_if_stopped;
		if (!LoadFromNative(terr))
		{
			/*return_if_stopped;
			GenerateTerrain();
			return_if_stopped;
			AutoGenSplatMap();*/
			return_if_stopped;
			std::lock_guard<std::mutex> lock(m_generatorMtx);
			// single size -1 so that map piece can be integrated
			const uint32_t single_max = m_stData.singleMapSize - 1;
			uint32_t wx_min = x_index * single_max;
			uint32_t wx_max = (x_index + 1)* single_max;
			uint32_t wy_min = y_index * single_max;
			uint32_t wy_max = (y_index + 1)* single_max;
			uint32_t idx = 0;
			uint32_t splat_idx = 0;
			int sizeAlpha = m_stData.splatCount;
			const uint32_t splatWidth = m_stData.splatWidth;
			LogFormat("copy terrain %d's data", terr);
			for (uint32_t wy = wy_min; wy <= wy_max; wy++)
			{
				for (uint32_t wx = wx_min; wx <= wx_max; wx++)
				{
					m_aHeightMapCopy[idx++] = m_worldMapHeightMap[GetHeightMapIndex(wx, wy)];
					for (uint32_t a = 0; a < sizeAlpha; a++)
					{
						m_aSplatMapCopy[splat_idx++] = m_aSplatMap[GetSplatMapIndex(wx, wy, a, splatWidth, sizeAlpha)];
					}
					/*if (wx == wx_min || wx == wx_max)
					{
						LogFormat("%d height at x %d,y %d is %f,idx0 %d,idx1 %d,totalsize %d", terr, wx, wy, m_aHeightMapCopy[idx], GetHeightMapIndex(wx, wy), idx, m_stData.singleMapSize*m_stData.singleMapSize);

					}*/
				}
			}
			LogFormat("copy terrain %d's data finish,size %d,perhabs %d,splat %d,max %d", terr, idx, m_stData.singleMapSize*m_stData.singleMapSize, splat_idx, splatWidth*splatWidth*sizeAlpha);
			
			m_mTerrainData.insert(std::make_pair(terr, m_pCurrentMap));
			m_finishQueue.add(terr);
			LogFormat("gen terrain %d finish ,current terrain count %d ", m_pCurrentMap->m_nInstanceId, m_mTerrainData.size());
		}
		else
		{
			//todo
		}
	}
	void MapGenerator::GenWorldMap()
	{
		setRandomSeed(m_stData.seed);
		uint32_t i = GetWorldMapSize();
		m_pGenerator->Initilize(0xffffffff, std::rand(), i, m_stData.H, m_worldMapHeightMap);
		m_pWorldMap = std::make_shared<Terrain>(0xffffffff, i, m_pGenerator->GetSquareSize());
		m_pWorldMap->Init(m_worldMapHeightMap, m_pGenerator->GetSquareSize());
		LogFormat("GenWorldMap size %d,i %d", m_pWorldMap->GetHeightMapSize(), m_pWorldMap->GetI());
		if (!LoadFromNative(0xffffffff) && m_pWorldMap->GetI() > 1)
		{
			float worldMapConor[] = { m_stData.height0,m_stData.height1,m_stData.height2,m_stData.height3 };
			m_pGenerator->Start(worldMapConor, 4, 0);
			return_if_stopped;
		}
		AutoGenSplatMap();
	}
	void MapGenerator::AutoGenSplatMap() const
	{
		LogFormat("Generate terrain %d splat map start", m_pWorldMap->m_nInstanceId);
		return_if_stopped;
		m_pPainter->DrawSplatMap();
		LogFormat("Generate terrain %d splat map finished", m_pWorldMap->m_nInstanceId);
	}

	void MapGenerator::GenerateTerrain() const
	{
		//setRandomSeed(time(nullptr));
		//static float cornor[] = { m_stData.height0,m_stData.height1,m_stData.height2,m_stData.height3 };
		return_if_stopped;
		//m_pGenerator->Start(cornor, 4, m_stData.flags & 0x4 ? m_pCurrentMap->GetRealSize() : 0);
		//LogFormat("Generate terrain %d height map finished", m_pCurrentMap->m_nInstanceId);
	}
	uint32_t MapGenerator::GetNeighborID(NeighborType dir, uint32_t who) const
	{
		if (who == 0)
		{
			return 0;
		}
		uint32_t worldMapSize = m_stData.worldMapSize;
		uint32_t index = who - 1;
		uint32_t x = index % m_stData.worldMapSize;
		uint32_t y = index / m_stData.worldMapSize;
		int32_t idx;
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
		}
	}
	bool MapGenerator::LoadFromNative(uint32_t terr) const
	{
		//todo load raw map
		return false;
	}

	bool MapGenerator::GetHeightOnNeighbor(int32_t x, int32_t y, NeighborType neighbor, uint32_t owner, float & p)
	{
		//return false;
		if (!instance || !instance->m_pCurrentMap || instance->m_pCurrentMap->m_nInstanceId != owner)
		{
			return false;
		}
		//owner is not added to m_mTerrainData yet,get it from m_pCurrentMap
		//LogWarningFormat("terrain %d generator get neighbor %d at x %d,y %d,map size %d", owner, neighbor, x, y,instance->m_pCurrentMap->m_nInstanceId);
		bool ret = instance->m_pCurrentMap->GetNeighborHeight(x, y, neighbor, p);
		return ret;
	}

	float MapGenerator::GetHeightOnWorldMap(int32_t cx, int32_t cy, uint32_t owner)
	{
		if (!instance)
		{
			LogError("World Generator is not initilized");
			return 0;
		}
		const float scale = instance->m_pCurrentMap->GetHeightMapSize()*instance->m_stData.worldMapSize / (float)instance->m_pWorldMap->GetHeightMapSize();
		const uint32_t mapIndex = owner - 1;
		const uint32_t x = mapIndex % instance->m_stData.worldMapSize;
		const uint32_t y = mapIndex / instance->m_stData.worldMapSize;
		float fx = x * instance->m_pCurrentMap->GetRealSize();
		float fy = y * instance->m_pCurrentMap->GetRealSize();
		const uint32_t offsetX = x * instance->m_stData.singleMapSize;
		const uint32_t offsetY = y * instance->m_stData.singleMapSize;

		float fwx = (cx + offsetX) / scale;
		float fwy = (cy + offsetY) / scale;
		const int wx = fwx;
		const int wy = fwy;
		fwx -= wx;
		fwy -= wy;
		const int wx_1 = std::ceilf(wx + fwx);
		const int wy_1 = std::ceilf(wy + fwy);
		const float height0 = instance->m_pWorldMap->GetHeight(wx, wy);
		float height1 = height0;
		if (wx_1 != wx && wy_1 != wy && wx_1 >= 0 && wx_1 < instance->m_pWorldMap->GetHeightMapSize() && wy_1 >= 0 && wy_1 < instance->m_pWorldMap->GetHeightMapSize())
		{
			height1 = instance->m_pWorldMap->GetHeight(wx_1, wy_1);
			//if x1 == x2,use fwy 
			if (wx_1 == wx)
			{
				fwx = fwy;
			}
		}
		return height0 * (1 - fwx) + height1 * fwx;
	}

	void MapGenerator::InitHeightMapBaseOnNeighbor(NeighborType position, std::shared_ptr<Terrain> neighbor) const
	{
		int m_nSize = m_stData.singleMapSize;
		int nMax = m_nSize - 1;
		//LogFormat("InitHeightMapBaseOnNeighbor pos %d,for terrain %d", position, m_pCurrentMap->m_nInstanceId);
		if (position == NeighborType::neighborPositionLeft)
		{
			if (m_pCurrentMap->m_pLeftNeighbor == neighbor)
			{
				//add left edge
				int x = 0;
				int nx(x + nMax);
				for (int y = 0; y <= nMax; y++)
				{
					float nheight = m_pCurrentMap->m_pLeftNeighbor->GetHeight(nx, y);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionRight)
		{
			if (m_pCurrentMap->m_pRightNeighbor == neighbor)
			{
				int x = nMax;
				int nx(x - nMax);
				for (int y = 0; y <= nMax; y++)
				{
					float nheight = m_pCurrentMap->m_pRightNeighbor->GetHeight(nx, y);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionBottom)
		{
			if (m_pCurrentMap->m_pBottomNeighbor == neighbor)
			{
				int y = 0;
				int nY(y + nMax);
				for (int x = 0; x <= nMax; x++)
				{
					float nheight = m_pCurrentMap->m_pBottomNeighbor->GetHeight(x, nY);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}

		if (position == NeighborType::neighborPositionTop)
		{
			if (m_pCurrentMap->m_pTopNeighbor == neighbor)
			{
				int y = nMax;
				int nY(y - nMax);
				for (int x = 0; x <= nMax; x++)
				{
					const float nheight = m_pCurrentMap->m_pTopNeighbor->GetHeight(x, nY);
					m_pGenerator->SetPulse(x, y, nheight);
				}
			}
		}
	}
}