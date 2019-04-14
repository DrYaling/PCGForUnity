#include "Diamond_Square.h"
#include "Mesh/UnityMesh.h"
#include "Logger/Logger.h"
namespace generator
{
	static G3D::Vector3 vector3_zero(0, 0, 0);
	using namespace G3D;
	Diamond_Square::Diamond_Square(int32_t seed, int32_t I, float H, /*std::vector<float>&*/float* heightMap) :
		m_nI(I),
		m_nH(H / 100.0f),
		m_bIsFinished(false),
		m_bEdgeExtended(false),
		m_vHeightMap(heightMap)
	{
		setRandomSeed(seed);
		m_nSize = std::pow(2, 2 * I) + 1;
		m_nMax = m_nSize - 1;
		m_nheightMapSize = m_nSize * m_nSize;

		//LogFormat("mesh count %d,nSize %d", meshCount, m_nSize);
	}

	Diamond_Square::~Diamond_Square()
	{
		m_cbProcessHandler = nullptr;
		m_vHeightMap = nullptr;
	}
	void Diamond_Square::Start(const float * corner, const int32_t size, std::function<void(void)> cb)
	{
		m_bIsFinished = false;
		if (size != 4 || nullptr == corner)
		{
			LogError("Diamond_Square Start Fail!");
			return;
		}
		size_t meshCount = GetMeshTheoreticalCount();
		if (meshCount > MAX_MESH_COUNT)
		{

			return;
		}
		auto itr = m_mExtendedMap.find(0);
		auto end = m_mExtendedMap.end();
		if (itr == end)
			SetAtXY(0, 0, corner[0] / MAX_MAP_HEIGHT);
		else
			SetAtXY(0, 0, itr->second);
		itr = m_mExtendedMap.find(GetHeightMapIndex(m_nMax, 0));
		if (itr == end)
			SetAtXY(m_nMax, 0, corner[1] / MAX_MAP_HEIGHT);
		else
			SetAtXY(m_nMax, 0, itr->second);
		itr = m_mExtendedMap.find(GetHeightMapIndex(0, m_nMax));
		if (itr == end)
			SetAtXY(0, m_nMax, corner[2] / MAX_MAP_HEIGHT);
		else
			SetAtXY(0, m_nMax, itr->second);
		itr = m_mExtendedMap.find(GetHeightMapIndex(m_nMax, m_nMax));
		if (itr == end)
			SetAtXY(m_nMax, m_nMax, corner[3] / MAX_MAP_HEIGHT);
		else
			SetAtXY(m_nMax, m_nMax, itr->second);
		//std::thread t(std::bind(&Diamond_Square::WorkThread, this,cb));
		//t.detach();
		//LogFormat("Diamond_Square Start,H %f,I %d,maxSize %d,meshCount %d", m_nH, m_nI, m_nSize, meshCount);
		WorkThread(cb);
	}

	void Diamond_Square::ReleaseUnusedBuffer()
	{
		for (auto itr = m_mExtendedMap.begin(); itr != m_mExtendedMap.end();)
		{
			itr = m_mExtendedMap.erase(itr);
		}
		release_map(m_mExtendedMap);
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
		//Blur();
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
			auto itr = m_mExtendedMap.find(GetHeightMapIndex(x, y));
			if (itr != m_mExtendedMap.end())
			{
				SetAtXY(x, y, itr->second);
				//LogWarningFormat("Diamond at x %d,y %d key %d extend found %f", x, y, x + y * m_nSize, itr->second);
				return;
			}
			else
			{
				//LogWarningFormat("Diamond at x %d,y %d  extend not found", x, y);
			}
		}
		float *p = m_aPointBuffer;// p0/*left*/, p1/*bottom*/, p2/*right*/, p3/*top*/;

		//four corner is excluded
		//so nigher x = 0 or x = max or y = 0 or y = max,but wont apear same time
		if (x - size < 0)
		{
			p[1] = GetAtXY(x, y - size);// m_vHeightMap[m_nSize * (y - size)];
			p[2] = GetAtXY(size, y);// m_vHeightMap[m_nSize*y + size];
			p[3] = GetAtXY(x, y + size);// m_vHeightMap[m_nSize*(y + size)];
			if (m_bEdgeExtended && m_cbGetNeighborVertice(x - size, y, NeighborType::neighborPositionLeft,p[4]))
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
			p[0] = GetAtXY(x - size, y);// m_vHeightMap[x - size + m_nSize * y];
			p[1] = GetAtXY(x, y - size);//m_vHeightMap[x + m_nSize * (y - size)];
			p[2] = p[3] = GetAtXY(x, y + size);// m_vHeightMap[x + m_nSize * (y + size)];
			if (m_bEdgeExtended && m_cbGetNeighborVertice(x + size, y, NeighborType::neighborPositionRight, p[4]))
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
			p[0] = GetAtXY(x - size, 0);// m_vHeightMap[x - size];
			p[2] = GetAtXY(x + size, 0);// m_vHeightMap[x + size];
			p[3] = GetAtXY(x, size);// m_vHeightMap[x + m_nSize * size];
			if (m_bEdgeExtended && m_cbGetNeighborVertice(x, y - size, NeighborType::neighborPositionBottom, p[4]))
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
			p[0] = GetAtXY(x - size, y);// m_vHeightMap[x - size + m_nSize * y];
			p[1] = GetAtXY(x, y - size);//m_vHeightMap[x + m_nSize * (y - size)];
			p[2] = GetAtXY(x + size, y);//m_vHeightMap[x + size + m_nSize * y];
			if (m_bEdgeExtended && m_cbGetNeighborVertice(x, y - size, NeighborType::neighborPositionRight, p[4]))
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
			p[0] = GetAtXY(x - size, y);//m_vHeightMap[x - size + m_nSize * y];
			p[1] = GetAtXY(x, y - size);//m_vHeightMap[x + m_nSize * (y - size)];
			p[2] = GetAtXY(x + size, y);//m_vHeightMap[x + size + m_nSize * y];
			p[3] = GetAtXY(x, y + size);//m_vHeightMap[x + m_nSize * (y + size)];
		}
		p[4] = (p[0] + p[1] + p[2] + p[3]) / 4.0f;
		SetAtXY(x, y, p[4] + h * p[4]);
		//if (x == m_nMax)
		//	LogFormat("diamond x %d,y %d,p %f,h %f,r %f,size %d", x, y, p[4], h, GetAtXY(x, y), size);
	}
	//正方形生成不用考虑边界条件
	inline void Diamond_Square::Square(int x, int y, int size, float h)
	{
		if (m_bEdgeExtended)
		{
			auto itr = m_mExtendedMap.find(GetHeightMapIndex(x, y));
			if (itr != m_mExtendedMap.end())
			{
				SetAtXY(x, y, itr->second);
				//LogWarningFormat("square at x %d,y %d  extend found %f", x, y, itr->second);
				return;
			}
			else
			{
				//LogWarningFormat("square at x %d,y %d  extend not found", x, y);
			}
		}
		m_aPointBuffer[4] = (
			GetAtXY(x - size, y - size) +
			GetAtXY(x + size, y - size) +
			GetAtXY(x - size, y + size) +
			GetAtXY(x + size, y + size)
			) / 4.0f;
		SetAtXY(x, y, m_aPointBuffer[4] + h * m_aPointBuffer[4]);
		//LogFormat("Square x %d,y %d,p %f,h %f,r %f", x, y, height, h, m_vHeightMap[x + m_nSize * y]);
	}

	inline float Diamond_Square::Randomize(float h)
	{
		return _frandom_f(-h, h);
	}

	void Diamond_Square::Blur()
	{
		for (size_t y = 0; y < m_nSize; y++)
		{
			for (size_t x = 0; x < m_nSize; x++)
			{
				SetBlurAtXY(x, y, GetAtXY(x, y));
			}
		}
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
				Vector3 p(GetDistance(x, maxCoord), GetAtXY(x, y), GetDistance(y, maxCoord));
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