#include "Diamond_Square.h"
#include "Mesh/UnityMesh.h"
#include "Logger/Logger.h"
#include "Noises/PerlinNoise.h"
namespace generator
{
	using namespace G3D;
	Diamond_Square::Diamond_Square() :
		m_vHeightMap(nullptr),
		m_nheightMapSize(0),
		m_cbGetNeighborHeight(nullptr),
		m_nSize(0),
		m_nH(0),
		m_nI(0),
		m_nMax(0), m_fDeltaSize(0),
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
		m_mExtendedMap.clear();
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
		m_fDeltaSize = mapWidth / (float)m_nSize;
		if (m_fDeltaSize < 0.001f)
		{
			m_fDeltaSize = 0.01f;
		}
		auto itr = m_mExtendedMap.find(0);
		auto end = m_mExtendedMap.end();
		if (itr == end)
			SetHeight(0, 0, corner[0] / MAX_MAP_HEIGHT);
		else
			SetHeight(0, 0, itr->second);
		itr = m_mExtendedMap.find(GetHeightMapIndex(m_nMax, 0));
		if (itr == end)
			SetHeight(m_nMax, 0, corner[1] / MAX_MAP_HEIGHT);
		else
			SetHeight(m_nMax, 0, itr->second);
		itr = m_mExtendedMap.find(GetHeightMapIndex(0, m_nMax));
		if (itr == end)
			SetHeight(0, m_nMax, corner[2] / MAX_MAP_HEIGHT);
		else
			SetHeight(0, m_nMax, itr->second);
		itr = m_mExtendedMap.find(GetHeightMapIndex(m_nMax, m_nMax));
		if (itr == end)
			SetHeight(m_nMax, m_nMax, corner[3] / MAX_MAP_HEIGHT);
		else
			SetHeight(m_nMax, m_nMax, itr->second);
		//std::thread t(std::bind(&Diamond_Square::WorkThread, this,cb));
		//t.detach();
		LogFormat("Diamond_Square Start,H %f,I %d,maxSize %d,extend %d", m_nH, m_nI, m_nSize, m_bEdgeExtended.load(std::memory_order_relaxed));
		WorkThread(cb);
	}

	void Diamond_Square::ReleaseUnusedBuffer()
	{
		for (auto itr = m_mExtendedMap.begin(); itr != m_mExtendedMap.end();)
		{
			itr = m_mExtendedMap.erase(itr);
		}
		release_map(m_mExtendedMap);
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
		Flush();
		while (generateSize > 0)
		{
			//generate square
			for (y = generateSize; y < m_nMax; y += prevSize)
			{
				for (x = generateSize; x < m_nMax; x += prevSize)
				{
					Square(x, y, generateSize, Randomize(_H));
					genLen++;
				}
			}
			//genrate diamond
			for (y = 0; y <= m_nMax; y += generateSize)
			{
				for (x = (y + generateSize) % prevSize; x <= m_nMax; x += prevSize)
				{
					Diamond(x, y, generateSize, Randomize(_H));
					genLen++;
				}
			}
			//LogFormat("process gen,current generate Size %d,H %f,process %f", generateSize, _H, process);
			//prepare for next generate
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
		//LogFormat("ds over,total size %d,should be %d", genLen, m_nSize*m_nSize - 4);
		Blur(m_fDeltaSize > 0);
		Blur();
		m_bIsFinished = true;
		if (cb)
		{
			cb();
		}
	}
	//������������߽�������򵥵ĴӷǱ߽��ȡһ�������߽�����
	inline void Diamond_Square::Diamond(int x, int y, int size, float h)
	{
		if (m_bEdgeExtended)
		{
			float height = 0;
			if (GetExtendedHeight(x, y, height))
			{
				SetHeight(x, y, height);
				//LogWarningFormat("Diamond at x %d,y %d key %d extend found %f", x, y, GetHeightMapIndex(x, y), height);
				return;
			}
			else
			{
				//LogWarningFormat("Diamond at x %d,y %d  extend not found", x, y);
			}
		}
		float m_aPointBuffer[5];
		float *p = m_aPointBuffer;// p0/*left*/, p1/*bottom*/, p2/*right*/, p3/*top*/;
		/*if (m_Owner != 0xffffffff && m_Owner > 1)
		{
			LogWarningFormat("owner %d Diamond %d,%d,%d", m_Owner, x, y, size);
		}*/
		//four corner is excluded
		//so nigher x = 0 or x = max or y = 0 or y = max,but wont apear same time
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
		SetHeight(x, y, p[4] + h * p[4]);
		//if (x == m_nMax)
		//	LogFormat("diamond x %d,y %d,p %f,h %f,r %f,size %d", x, y, p[4], h, GetHeight(x, y), size);
	}
	//���������ɲ��ÿ��Ǳ߽�����
	inline void Diamond_Square::Square(int x, int y, int size, float h)
	{
		if (m_bEdgeExtended)
		{
			float height = 0;
			if (GetExtendedHeight(x, y, height))
			{
				SetHeight(x, y, height);
				//LogWarningFormat("Diamond at x %d,y %d key %d extend found %f", x, y, x + y * m_nSize, itr->second);
				return;
			}
			else
			{
				//LogWarningFormat("square at x %d,y %d  extend not found", x, y);
			}
		}
		float p;
		p = (
			GetHeight(x - size, y - size) +
			GetHeight(x + size, y - size) +
			GetHeight(x - size, y + size) +
			GetHeight(x + size, y + size)
			) / 4.0f;
		SetHeight(x, y, p + h * p);
		//LogFormat("Square x %d,y %d,p %f,h %f,r %f", x, y, height, h, m_aHeightMap[x + m_nSize * y]);
	}

	inline float Diamond_Square::Randomize(float h)
	{
		return _frandom_f(-h, h);
	}

	void Diamond_Square::Flush()
	{
		//flush map 
		//first flush left edge
		// choose 2 points if exist
		uint32_t x = 0;

		float height0 = 0;
		float height1 = 0;
		float height2 = 0;
		uint32_t x1, x2, x3, y1, y2, y3;
		//we use a ray to caculate third point
		//if fist point is not set,do not flush this map
		//ax1+b = y1
		//ax2+b = y2
		//y3 = ?
		//a = (y1-y2)/(x1-x2)
		//b = (x1y2-x2y1)/(x1-x2)
		//so y3 = x3*a+b =x3*(y1-y2)/(x1-x2)+(x1y2-x2y1)/(x1-x2)
		if (!GetExtendedHeight(0, 0, height0))
		{
			return;
		}
		if (!GetExtendedHeight(0, m_nMax, height1))
		{
			//if max y not set,set it
			if (m_mExtendedMap.size() >= 8)//insure valid map,if extend size is too small,this map may be too small
			{
				height1 = height0 + Randomize(m_nH)*height0;
				for (uint32_t y = m_nMax - 1; y >= 0; y++)
				{
					if (GetExtendedHeight(x, y, height1))
					{
						//here x1 = 0,x2 = y,x3 = m_nMax
						y1 = height0;
						y2 = height1;
						x1 = 0;
						x2 = y;
						x3 = m_nMax;
						y3 = (x3*(y1 - y2) + x1 * y2 - x2 * y1) / (float)(x1 - x2);
						height1 = y3 /*additianal randomize height*/ + (height1 - height0)*Randomize(m_nH);
						break;
					}
				}
				SetPulse(x, m_nMax, height1);
			}
			else
			{
				return;
			}
		}
		uint32_t y0 = 0;
		for (uint32_t y = 0; y < m_nSize; y++)
		{
			//if this point is not set,find next point to flush it
			if (!GetExtendedHeight(x, y, height1))
			{
				y1 = 0xffffffff;
				height2 = (float&)y1;
				for (uint32_t ey = y + 1; ey < m_nSize; ey++)
				{
					if (GetExtendedHeight(x, ey, height2))
					{
						//here x1 = y0,x2 = ey,x3 = y
						//y1 = height0,y2 = height2
						y1 = height0;
						y2 = height2;
						x1 = y0;
						x2 = ey;
						x3 = y;
						y3 = (x3*(y1 - y2) + x1 * y2 - x2 * y1) / (float)(x1 - x2);
						height2 = y3 /*additianal randomize height*/ + (height2 - height0)*Randomize(m_nH) / 2.f;
						break;
					}
				}
				//do not find set point,break(no more will be found)
				LogFormat("df");
				if (0xffffffff == (uint32_t&)height2)
				{
					LogFormat("flush break at x %d,y %d", 0, y);
					break;;
				}
				else
				{
					SetPulse(x, y, height2);
					height0 = height1;
					y0 = y;
				}
			}
		}
		LogFormat("%d extended point after flush state 1", m_mExtendedMap.size() - m_nMax);
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
		//��Ϊÿһ��mesh����ʼy��������һ��mesh���յ�y���꣬���Ժ����mesh�ڼ���offset��ʱ����Ҫ���һ��m_nSize���������һ��m_nSize�Ĳ�ֵ
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