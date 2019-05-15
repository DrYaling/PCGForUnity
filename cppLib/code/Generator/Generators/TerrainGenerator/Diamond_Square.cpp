#include "Diamond_Square.h"
#include "Mesh/UnityMesh.h"
#include "Logger/Logger.h"
#include "Noises/PerlinNoise.h"
#include "MapGenerator/MapGenerator.h"
namespace generator
{
#define return_if_exit if(MapGenerator::IsStopped()) return 
	using namespace G3D;
	Diamond_Square::Diamond_Square() :
		m_Owner(0),
		m_vHeightMap(nullptr),
		m_nheightMapSize(0),
		m_cbGetNeighborHeight(nullptr),
		m_cbGetWorldHeight(nullptr),
		m_nSize(0),
		m_nH(0),
		m_nI(0),
		m_nFlushDepth(0),
		m_nMax(0),
		m_fDeltaSize(0),
		m_bIsFinished(false),
		m_bEdgeExtended(false), m_bInitilized(false)
	{
		//LogFormat("mesh count %d,nSize %d", meshCount, m_nSize);
	}

	Diamond_Square::~Diamond_Square()
	{
		m_cbProcessHandler = nullptr;
		m_vHeightMap = nullptr;
	}
	void Diamond_Square::Initilize(uint32_t owner, int32_t seed, int32_t I, float H, float * heightMap)
	{
		m_nI = I;
		m_nH = H / 100.0f;
		m_bIsFinished = false;
		//m_bEdgeExtended = false;
		m_vHeightMap = heightMap;
		setRandomSeed(seed);
		m_nSize = std::pow(2, 2 * I) + 1;
		m_nMax = m_nSize - 1;
		m_nheightMapSize = m_nSize * m_nSize;
		m_bInitilized = true;
		m_Owner = owner;
		LogFormat("generator initilize owner %d,extended %d", owner, m_bEdgeExtended.load(std::memory_order_relaxed));
	}
	void Diamond_Square::Reset()
	{
		m_bIsFinished = false;
		m_bEdgeExtended = false;
		m_mWeightMap.clear();
	}
	void Diamond_Square::Start(const float * corner, const int32_t size, int32_t mapWidth, std::function<void(void)> cb)
	{
		m_bIsFinished = false;
		if (size != 4 || nullptr == corner)
		{
			LogError("Diamond_Square Start Fail!");
			return;
		}
		/*size_t meshCount = GetMeshTheoreticalCount();
		if (meshCount > MAX_MESH_COUNT)
		{

			return;
		}*/
		LogFormat("start generate");
		m_fDeltaSize = mapWidth / static_cast<float>(m_nSize);
		if (m_fDeltaSize < 0.001f)
		{
			m_fDeltaSize = -0.01f;
		}
		if (m_Owner == 0xffffffff)
		{
			auto itr = m_mWeightMap.find(0);
			const auto end = m_mWeightMap.end();
			if (itr == end)
				SetHeight(0, 0, corner[0] / MAX_MAP_HEIGHT);
			else
				SetHeight(0, 0, itr->second);
			itr = m_mWeightMap.find(GetHeightMapIndex(m_nMax, 0));
			if (itr == end)
				SetHeight(m_nMax, 0, corner[1] / MAX_MAP_HEIGHT);
			else
				SetHeight(m_nMax, 0, itr->second);
			itr = m_mWeightMap.find(GetHeightMapIndex(0, m_nMax));
			if (itr == end)
				SetHeight(0, m_nMax, corner[2] / MAX_MAP_HEIGHT);
			else
				SetHeight(0, m_nMax, itr->second);
			itr = m_mWeightMap.find(GetHeightMapIndex(m_nMax, m_nMax));
			if (itr == end)
				SetHeight(m_nMax, m_nMax, corner[3] / MAX_MAP_HEIGHT);
			else
				SetHeight(m_nMax, m_nMax, itr->second);
		}
		//std::thread t(std::bind(&Diamond_Square::WorkThread, this,cb));
		//t.detach();
		LogFormat("Diamond_Square Start,H %f,I %d,maxSize %d,extend %d", m_nH, m_nI, m_nSize, m_bEdgeExtended.load(std::memory_order_relaxed));
		WorkThread(cb);
	}

	void Diamond_Square::ReleaseUnusedBuffer()
	{
		for (auto itr = m_mWeightMap.begin(); itr != m_mWeightMap.end();)
		{
			itr = m_mWeightMap.erase(itr);
		}
		release_map(m_mWeightMap);
		m_bInitilized = false;
	}

	void Diamond_Square::WorkThread(std::function<void(void)> cb)
	{
		float _H = m_nH;
		float process = 0;//max 100
		int Iprocess = 0;
		float processSpeedPerWhile = 1 / std::log2f(m_nMax) * 100.0f;
		//LogFormat("processSpeedPerWhile %f", processSpeedPerWhile);
		int generateSize = m_nMax / 2;
		int prevSize = m_nMax;
		int genLen = 0;
		int x, y;
		if (m_Owner != 0xffffffff)
			Flush();
		while (generateSize > 0)
		{
			return_if_exit;
			//generate square
			for (y = generateSize; y < m_nMax; y += prevSize)
			{
				for (x = generateSize; x < m_nMax; x += prevSize)
				{
					return_if_exit;
					Square(x, y, generateSize, Randomize(_H));
					genLen++;
					if (x %64 ==0 && y % 64 == 0)
					{
						LogWarningFormat("% height at x %d,y %d is %f",m_Owner,x,y,GetHeight(x,y));
					}
				}
			}
			//genrate diamond
			for (y = 0; y <= m_nMax; y += generateSize)
			{
				for (x = (y + generateSize) % prevSize; x <= m_nMax; x += prevSize)
				{
					return_if_exit;
					Diamond(x, y, generateSize, Randomize(_H));
					if (x % 64 == 0 && y % 64 == 0)
					{
						LogWarningFormat("% height at x %d,y %d is %f", m_Owner, x, y, GetHeight(x, y));
					}
					genLen++;
				}
			}
			//LogFormat("process gen,current generate Size %d,H %f,process %f", generateSize, _H, process);
			//prepare for next generate
			m_nFlushDepth--;
			prevSize = generateSize;
			generateSize /= 2;
			_H /= 2.0f;
			process += processSpeedPerWhile;
			int pro = std::floor(process);
			//process call
			if (pro > Iprocess)
			{
				if (m_cbProcessHandler)
				{
					m_cbProcessHandler(pro);
				}
			}
			Iprocess = pro;
		}
		LogFormat("ds over,total size %d,should be %d", genLen, m_nSize*m_nSize - 4);
		Blur(m_fDeltaSize > 0);
		Blur();
		m_bIsFinished = true;
		if (cb)
		{
			cb();
		}
	}
	//菱形如果遇到边界情况，简单的从非边界点取一个来做边界数据
	inline void Diamond_Square::Diamond(int x, int y, int size, float h)
	{
		if (m_bEdgeExtended)
		{
			float height = 0;
			if (GetWeightMapHeight(x, y, height))
			{
				SetHeight(x, y, height);
				return;
			}
		}
		float m_aPointBuffer[5];
		float *p = m_aPointBuffer;// p0/*left*/, p1/*bottom*/, p2/*right*/, p3/*top*/;
		if (x - size < 0)
		{
			p[1] = GetHeight(x, y - size);// m_aHeightMap[m_nSize * (y - size)];
			p[2] = GetHeight(size, y);// m_aHeightMap[m_nSize*y + size];
			p[3] = GetHeight(x, y + size);// m_aHeightMap[m_nSize*(y + size)];
			if (GetHeightOnWorldMap(x - size, y, NeighborType::neighborPositionLeft, p[4]))
			{
				p[0] = p[4];
			}
			else
			{
				p[0] = p[_irandom(1, 3)];
			}
		}
		else if (x + size > m_nMax)
		{
			p[0] = GetHeight(x - size, y);// m_aHeightMap[x - size + m_nSize * y];
			p[1] = GetHeight(x, y - size);//m_aHeightMap[x + m_nSize * (y - size)];
			p[2] = p[3] = GetHeight(x, y + size);// m_aHeightMap[x + m_nSize * (y + size)];
			if (GetHeightOnWorldMap(x + size, y, NeighborType::neighborPositionRight, p[4]))
			{
				p[2] = p[4];
			}
			else
			{
				int i = _irandom(0, 2);
				if (i == 2)
				{
					i++;
				}
				p[2] = p[i];
				//LogFormat("at x %d,y %d ,no neighbor or edge extended %f",x,y, p[2]);
			}
		}
		else if (y - size < 0)
		{
			p[0] = GetHeight(x - size, 0);// m_aHeightMap[x - size];
			p[2] = GetHeight(x + size, 0);// m_aHeightMap[x + size];
			p[3] = GetHeight(x, size);// m_aHeightMap[x + m_nSize * size];
			if (GetHeightOnWorldMap(x, y - size, NeighborType::neighborPositionBottom, p[4]))
			{
				p[1] = p[4];
			}
			else
			{
				int i = _irandom(1, 3);
				if (i == 1)
				{
					i--;
				}
				p[1] = p[i];
			}
		}
		else if (y + size > m_nMax)
		{
			p[0] = GetHeight(x - size, y);// m_aHeightMap[x - size + m_nSize * y];
			p[1] = GetHeight(x, y - size);//m_aHeightMap[x + m_nSize * (y - size)];
			p[2] = GetHeight(x + size, y);//m_aHeightMap[x + size + m_nSize * y];
			if (GetHeightOnWorldMap(x, y - size, NeighborType::neighborPositionRight, p[4]))
			{
				p[3] = p[4];
			}
			else
			{
				p[3] = p[_irandom(0, 2)];
			}
		}
		else
		{
			p[0] = GetHeight(x - size, y);//m_aHeightMap[x - size + m_nSize * y];
			p[1] = GetHeight(x, y - size);//m_aHeightMap[x + m_nSize * (y - size)];
			p[2] = GetHeight(x + size, y);//m_aHeightMap[x + size + m_nSize * y];
			p[3] = GetHeight(x, y + size);//m_aHeightMap[x + m_nSize * (y + size)];
		}
		p[4] = (p[0] + p[1] + p[2] + p[3]) / 4.0f;
		if (m_bEdgeExtended)
		{
			float height = 0;
			if (GetWeightMapHeight(x, y, height))
			{
				p[4] = (p[4] + height) / 2.0f;
			}
		}
		SetHeight(x, y, p[4] + h * p[4]);
		//if (x == m_nMax)
		//	LogFormat("diamond x %d,y %d,p %f,h %f,r %f,size %d", x, y, p[4], h, GetHeight(x, y), size);
	}
	//正方形生成不用考虑边界条件
	inline void Diamond_Square::Square(int x, int y, int size, float h)
	{
		if (m_bEdgeExtended)
		{
			float height = 0;
			if (GetWeightMapHeight(x, y, height))
			{
				SetHeight(x, y, height);
				return;
			}
		}
		float p = (
			GetHeight(x - size, y - size) +
			GetHeight(x + size, y - size) +
			GetHeight(x - size, y + size) +
			GetHeight(x + size, y + size)
			) / 4.0f;
		if (m_bEdgeExtended)
		{
			float height = 0;
			if (GetWeightMapHeight(x, y, height))
			{
				p = (p + height) / 2.0f;
			}
		}
		SetHeight(x, y, p + h * p);
		//LogFormat("Square x %d,y %d,p %f,h %f,r %f", x, y, height, h, m_aHeightMap[x + m_nSize * y]);
	}

	inline float Diamond_Square::Randomize(float h)
	{
		return _frandom_f(-h, h);
	}
	//we use a ray to caculate third point
	//if fist point is not set,do not flush this map
	//ax1+b = y1
	//ax2+b = y2
	//y3 = ?
	//a = (y1-y2)/(x1-x2)
	//b = (x1y2-x2y1)/(x1-x2)
	//so y3 = x3*a+b =x3*(y1-y2)/(x1-x2)+(x1y2-x2y1)/(x1-x2)
	static float Function_line(uint32_t x1, uint32_t x2, uint32_t x3, float y1, float y2, float h)
	{
		float y3 = (x3*(y1 - y2) + x1 * y2 - x2 * y1) / (float)(x1 - x2);
		return y3 + (y2 - y1)*_frandom_f(-h, h);
	}
	//regard x3 as middle point between x1 and x2
	static float __fastcall Function(uint32_t x1, uint32_t x2, uint32_t x3, float y1, float y2)
	{
		float dis = x2 - x1;
		return  (y1*(x3 - x1) + y2 * (x2 - x3)) / 2.0f / dis;
		//h = h * (dis) / hScale;
		//return y3 + y3 * _frandom_f(-h, h);
	}
	void Diamond_Square::Flush()
	{
		return;
		//flush map 
		//first flush left edge
		// choose 2 points if exist
		uint32_t x = 0;

		float height0 = 0;
		float height1 = 0;
		float height2 = 0;
		uint32_t x1, x2, x3, y1, y2, y3;
		if (!GetWeightMapHeight(0, 0, height0))
		{
			return;
		}
		logger::ProfilerStart("generator flush");
		if (!GetWeightMapHeight(0, m_nMax, height1))
		{
			//if max y not set,set it
			if (m_mWeightMap.size() >= 8)//insure valid map,if extend size is too small,this map may be too small
			{
				height1 = height0 + Randomize(m_nH)*height0;
				for (uint32_t y = m_nMax - 1; y >= 0; y++)
				{
					if (GetWeightMapHeight(x, y, height1))
					{
						//here x1 = 0,x2 = y,x3 = m_nMax
						y1 = height0;
						y2 = height1;
						x1 = 0;
						x2 = y;
						x3 = m_nMax;
						height1 = Function(x1, x2, x3, y1, y2);
						break;
					}
				}
				SetPulse(x, m_nMax, height1 + height1 * Randomize(m_nH*(x3 - x1) / (float)m_nSize));
			}
			else
			{
				LogErrorFormat("weight map externed is not enough!");
				logger::ProfilerEnd();
				return;
			}
		}

		//left edge
		if (!SpreadWeightMap(0, 0, m_nSize))
		{
			LogErrorFormat("calculate left edge map fail");
			logger::ProfilerEnd();
			return;
		}
		if (m_mWeightMap.size() - m_nMax <= 4)
		{
			LogWarningFormat("%d extended point not enough to gen extro heights,ignore", m_Owner, m_mWeightMap.size() - m_nMax);
			logger::ProfilerEnd();
			return;
		}
		LogFormat("%d extended point after flush state 1", m_mWeightMap.size() - m_nMax);
		//second step
		//calculate right edge 
		//parse 1 ,calculate right point with whose has at least 2 point(x=0 included) in the same line
		std::map<int32_t, float> rightEdge;
		std::map<int32_t, float>::iterator itr = m_mWeightMap.begin();
		for (; itr != m_mWeightMap.end(); ++itr)
		{
			uint32_t index = itr->first;
			uint32_t ex = index % m_nSize;
			uint32_t ey = index / m_nSize;
			//find a point not in right edge
			if (ex > 0 && ex < m_nMax)
			{
				//to see if right point at this line exist
				auto exist = m_mWeightMap.find(GetHeightMapIndex(m_nMax, ey));
				if (exist == m_mWeightMap.end())
				{
					//if right edge point does not exist,set it base on left edge point and current point
					y1 = m_mWeightMap[GetHeightMapIndex(0, ey)];
					y2 = itr->second;
					x1 = 0;
					x2 = ey;
					x3 = m_nMax;
					height0 = Function(x1, x2, x3, y1, y2);
					rightEdge.insert(std::make_pair(GetHeightMapIndex(m_nMax, ey), height0 + height0 * Randomize(m_nH*(x2 - x1) / (float)m_nSize)));

				}
			}
			else if (ex == m_nMax)
			{
				rightEdge.insert(std::make_pair(GetHeightMapIndex(m_nMax, ey), m_mWeightMap[GetHeightMapIndex(m_nMax, ey)]));
			}
		}
		for (auto itrR = rightEdge.begin(); itrR != rightEdge.end(); ++itrR)
		{
			m_mWeightMap.insert(std::make_pair(itrR->first, itrR->second));
		}

		rightEdge.clear();
		for (uint32_t y = 0; y <= m_nMax; y += m_nMax)
		{
			//parse 2,calculate right bottom and right top
			if (!GetWeightMapHeight(m_nMax, y, height0))
			{
				int32_t find_count = 0;
				const int32_t offset = y == 0 ? 1 : -1;
				uint32_t ex0 = 0, ex1 = 0;
				for (uint32_t ey = y; ey >= 0 && ey <= m_nMax; ey += offset)
				{
					if (GetWeightMapHeight(m_nMax, ey, height2))
					{
						if (find_count == 0)
						{
							height1 = height2;
							ex0 = ey;
						}
						else
						{
							ex1 = ey;
						}
						find_count++;
						if (find_count >= 2)
						{
							break;
						}
					}
				}
				if (find_count != 2)
				{
					LogErrorFormat("cant find 2 point to calculate height at right edge of y %d", y);
					logger::ProfilerEnd();
					return;
				}
				y1 = height1;
				y2 = height2;
				x1 = ex0;
				x2 = ex1;
				x3 = y;
				height0 = Function(x1, x2, x3, y1, y2);
				m_mWeightMap.insert(std::make_pair(GetHeightMapIndex(m_nMax, y), height0 + height0 * Randomize(m_nH*(x2 - x1) / (float)m_nSize)));
			}
		}
		//parse 3,calculate right edge weight map
		if (!SpreadWeightMap(m_nMax, 0, m_nSize))
		{
			LogErrorFormat("calculate right edge map fail");
			logger::ProfilerEnd();
			return;
		}
		for (uint32_t y = 0; y < m_nSize; y++)
		{
			for (x = 1; x < m_nMax; x++)
			{
				SpreadWeightMapAreaWeight(y, x, m_nSize, m_nI);
			}
		}
		//third step
		//calculate weight map
		logger::ProfilerEnd();
	}

	bool Diamond_Square::SpreadWeightMap(uint32_t x, uint32_t ymin, uint32_t ymax)
	{
		float height0 = 0;
		float height1 = 0;
		float height2 = 0;
		uint32_t x1, x2, x3, y0 = ymin, y1, y2, y3;
		if (!GetWeightMapHeight(x, ymin, height0))
		{
			return  false;
		}
		for (uint32_t y = ymin + 1; y < ymax - 1; y++)
		{
			if (!GetWeightMapHeight(x, y, height1))
			{
				bool found = false;
				for (uint32_t ey = y + 1; ey < ymax; ey++)
				{
					if (GetWeightMapHeight(x, ey, height2))
					{
						//here x1 = y0,x2 = ey,x3 = y
						//y1 = height0,y2 = height2
						y1 = height0;
						y2 = height2;
						x1 = y0;
						x2 = ey;
						x3 = y;
						height2 = Function(x1, x2, x3, y1, y2);
						found = true;
						break;
					}
				}
				//do not find set point,break(no more will be found)
				if (!found)
				{
					LogFormat("flush break at x %d,y %d", 0, y);
					break;;
				}
				SetPulse(x, y, height2 + height2 * Randomize(m_nH*(x2 - x1) / (float)m_nSize));
				height0 = height1;
				y0 = y;
			}
		}
		return true;
	}

	bool Diamond_Square::SpreadWeightMapAreaWeight(uint32_t y, uint32_t xmin, uint32_t xmax, int32_t size)
	{
		float height0 = 0;
		float height1 = 0;
		float height2 = 0;
		uint32_t x1(xmin), x2(xmin), x3(xmin), x0 = xmin, y1 = 0, y2 = 0;
		if (!GetWeightMapHeight(xmin, y, height0))
		{
			return  false;
		}
		for (uint32_t x = xmin + 1; x < xmax - 1; x++)
		{
			if (!GetWeightMapHeight(x, y, height1))
			{
				bool found = false;
				for (uint32_t ex = x + 1; ex < xmax; ex++)
				{
					if (GetWeightMapHeight(ex, y, height2))
					{
						//here x1 = y0,x2 = ey,x3 = y
						//y1 = height0,y2 = height2
						y1 = height0;
						y2 = height2;
						x1 = x0;
						x2 = ex;
						x3 = y;
						height2 = Function(x1, x2, x3, y1, y2);
						found = true;
						break;
					}
				}
				//do not find set point,break(no more will be found)
				if (!found)
				{
					LogFormat("flush break at x %d,y %d", 0, y);
					break;;
				}
				//find extro height with size 
				int32_t count = 1;
				float extroHeight = 0;
				for (int32_t h = -size; h <= size; h++)
				{
					for (int32_t w = -size; w <= size; w++)
					{
						if (h == 0 && w == 0)
						{
							continue;
						}
						uint32_t hy = y + h;
						uint32_t wx = x + w;
						if (hy >= 0 && hy <= m_nMax && wx >= 0 && wx <= m_nMax && GetWeightMapHeight(wx, hy, height0))
						{
							extroHeight += height0;
							count++;
						}
					}
				}
				height2 = (height2 + extroHeight) / static_cast<float>(count);
				SetPulse(x, y, height2 + height2 * Randomize(m_nH*(x2 - x1) / (float)m_nSize));
				height0 = height1;
				x0 = x;
			}
		}
		return true;
	}

	void Diamond_Square::Blur(bool perlin) const
	{
		for (size_t y = 1; y < m_nMax; y++)
		{
			for (size_t x = 1; x < m_nMax; x++)
			{
				if (perlin)
				{
					const float height = GetHeight(x, y);
					float noise = PerlinNoise::noise(x*m_fDeltaSize, y*m_fDeltaSize, height);
					generator_clamp(noise, -0.5f, 0.5f);
					SetHeight(x, y, height*(1 + noise));
				}
				Smooth(x, y);
			}
		}
	}

	void Diamond_Square::Smooth(int32_t x, int32_t y) const
	{
		float h = 0.0F;
		h += GetHeight(x, y);
		h += GetHeight(x + 1, y);
		h += GetHeight(x - 1, y);
		h += GetHeight(x + 1, y + 1) * 0.75F;
		h += GetHeight(x - 1, y + 1) * 0.75F;
		h += GetHeight(x + 1, y - 1) * 0.75F;
		h += GetHeight(x - 1, y - 1) * 0.75F;
		h += GetHeight(x, y + 1);
		h += GetHeight(x, y - 1);
		h /= 8.0F;
		SetHeight(x, y, h);
	}


#if TERRAIN_GENERATE_VERTICES
	void Diamond_Square::GenerateTerrian(float maxCoord)
	{
		if (maxCoord <= 0)
		{
			return;
		}
		m_fDeltaSize = maxCoord / (float)m_nMax;
		int vidx = 0;
		for (int y = 0; y <= m_nMax; y++)
		{
			for (int x = 0; x <= m_nMax; x++)
			{
				Vector3 p(GetDistance(x, maxCoord), GetHeight(x, y), GetDistance(y, maxCoord));
				m_vVertices[vidx++] = p;
			}
		}
		vidx = 0;
		for (int y = 0; y <= m_nMax; y++)
		{
			for (int x = 0; x <= m_nMax; x++)
			{
				auto p = m_vVertices[vidx];
				pNeibor[0] = GetRealVertice(x - 1, y);
				pNeibor[1] = GetRealVertice(x, y - 1);
				pNeibor[2] = GetRealVertice(x + 1, y);
				pNeibor[3] = GetRealVertice(x, y + 1);
				m_stNormalBuffer = vector3_zero;
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[0], p - pNeibor[1]);
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[1], p - pNeibor[2]);
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[2], p - pNeibor[3]);
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[3], p - pNeibor[0]);
				m_vNormals[vidx++] = unityMesh::normalize(m_stNormalBuffer);
			}
		}
	}
	void Diamond_Square::RecaculateNormal(G3D::Vector3 * pN, int32_t size, int32_t mesh, int32_t position)
	{
		int xmin, xmax, ymin, ymax;
		switch (position)
		{
			case neighborPositionLeft:
			case  neighborPositionRight:
				{
					int meshCount = GetMeshTheoreticalCount();
					int vertexPerMesh = m_nMax / meshCount;
					int upBound = (mesh + 1)*vertexPerMesh;
					if (upBound > m_nMax)
					{
						upBound = m_nMax;
					}
					int start = mesh * vertexPerMesh;
					ymin = start;
					ymax = upBound;
					if (position == neighborPositionRight)
					{
						xmin = xmax = m_nMax;
					}
					else
					{
						xmin = xmax = 0;
					}
				}
				break;
			case neighborPositionBottom:
				xmin = 0;
				xmax = m_nMax;
				ymin = ymax = 0;
				break;
			case  neighborPositionTop:
				xmin = 0;
				xmax = m_nMax;
				ymin = ymax = m_nMax;
				break;
			default:
				break;
		}
		int offset = 0;
		//因为每一个mesh的起始y坐标是上一个mesh的终点y坐标，所以后面个mesh在减掉offset的时候需要多加一个m_nSize，否则会有一个m_nSize的差值
		for (size_t i = 0; i < mesh; i++)
		{
			//LogFormat("vertices size of mesh %d is %d", i, m_vVerticesSize[i]);
			offset += m_vVerticesSize[i] - m_nSize;
		}
		//LogFormat("RecaculateNormal mesh %d,size %d,position %d,xmin %d,xmax %d,ymin %d,ymax %d,offset %d", mesh, size, position, xmin, xmax, ymin, ymax, offset);
		int indexSize = 0;
		for (int y = ymin; y <= ymax; y++)
		{
			for (int x = xmin; x <= xmax; x++)
			{
				indexSize = x + y * m_nSize - offset;
				if (indexSize >= size)
				{
					LogErrorFormat("at mesh %d, x %d,y %d recaculate normal fail,normal size %d,indexSize %d", mesh, x, y, size, indexSize);
					return;
				}
				Vector3 p = GetRealVertice(x, y);
				pNeibor[0] = GetRealVertice(x - 1, y);
				pNeibor[1] = GetRealVertice(x, y - 1);
				pNeibor[2] = GetRealVertice(x + 1, y);
				pNeibor[3] = GetRealVertice(x, y + 1);

				m_stNormalBuffer = vector3_zero;
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[0], p - pNeibor[1]);
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[1], p - pNeibor[2]);
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[2], p - pNeibor[3]);
				m_stNormalBuffer += -unityMesh::getNormal(p - pNeibor[3], p - pNeibor[0]);
				pN[indexSize] = unityMesh::normalize(m_stNormalBuffer);
			}
		}
		//LogWarningFormat("at mesh %d,recaculate normal size %d,indexSize %d", mesh, size, indexSize);
}
#endif
}