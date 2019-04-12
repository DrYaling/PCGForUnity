#include "Diamond_Square.h"
#include "Mesh/UnityMesh.h"
#include "Logger/Logger.h"
namespace generator
{

	using namespace G3D;
	Diamond_Square::Diamond_Square(int32_t seed, int32_t I, float H, std::vector<float>& heightMap) :
		m_nI(I),
		m_nH(H / 100.0f),
		m_bIsFinished(false),
		m_bEdgeExtended(false),
		m_vHeightMap(heightMap)
	{
		setRandomSeed(seed);
		m_nSize = std::pow(2, 2 * I) + 1;
		m_nMax = m_nSize - 1;
		m_vHeightMap.resize(m_nSize*m_nSize);
		m_vExtendPoints.resize((m_nSize + 2)*(m_nSize + 2));
		m_vVertices.resize(m_vHeightMap.size());
		m_vNormals.resize(m_vHeightMap.size());

		size_t meshCount = GetMeshTheoreticalCount();
		int idx = 0;
		int startY(0);
		int nMax = m_nSize - 1;
		int	 outBoundY(nMax);
		int obY(nMax);
		outBoundY = obY = nMax / meshCount;
		//int32_t*** triangles;
		while (idx < meshCount)
		{
			int32_t verticesCount = m_nSize * (outBoundY - startY + 1);
			//LogFormat("mesh %d size %d,u %d,s %d", idx, verticesCount, outBoundY, startY);
			m_vVerticesSize.push_back(verticesCount);
			idx++;
			if (idx >= meshCount && outBoundY < m_nMax)
			{
				meshCount++;
			}
			startY = outBoundY;//因为最上面和最右边一排不计算三角形，所以在交界处需要多计算一次
			outBoundY += obY;
			if (outBoundY > m_nMax)
			{
				outBoundY = m_nMax;
			}
		}
		//LogFormat("mesh count %d,nSize %d", meshCount, m_nSize);
	}

	Diamond_Square::~Diamond_Square()
	{
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
			SetAtXY(0, 0, corner[0]);
		else
			SetAtXY(0, 0, itr->second);
		itr = m_mExtendedMap.find(m_nMax);
		if (itr == end)
			SetAtXY(m_nMax, 0, corner[1]);
		else
			SetAtXY(m_nMax, 0, itr->second);
		itr = m_mExtendedMap.find(m_nSize*m_nMax);
		if (itr == end)
			SetAtXY(0, m_nMax, corner[2]);
		else
			SetAtXY(0, m_nMax, itr->second);
		itr = m_mExtendedMap.find(m_nMax + m_nMax * m_nSize);
		if (itr == end)
			SetAtXY(m_nMax, m_nMax, corner[3]);
		else
			SetAtXY(m_nMax, m_nMax, itr->second);
		//std::thread t(std::bind(&Diamond_Square::WorkThread, this,cb));
		//t.detach();
		//LogFormat("Diamond_Square Start,H %f,I %d,maxSize %d,meshCount %d", m_nH, m_nI, m_nSize, meshCount);
		WorkThread(cb);
	}

	void Diamond_Square::ReleaseUnusedBuffer()
	{
		m_vNormals.clear();
		m_vVertices.clear();
		m_mExtendedMap.clear();
		//m_vVerticesSize.clear();
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
		/*for (auto f : m_vHeightMap)
		{
			if (f > -1 && f < 1)
				printf_s("%2f ", f);
		}
		printf_s("\n");*/
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
			auto itr = m_mExtendedMap.find(x + y * m_nSize);
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
		if (x == 0)
		{
			p[1] = GetAtXY(0, y - size);// m_vHeightMap[m_nSize * (y - size)];
			p[2] = GetAtXY(size, y);// m_vHeightMap[m_nSize*y + size];
			p[3] = GetAtXY(0, y + size);// m_vHeightMap[m_nSize*(y + size)];
			if (m_bEdgeExtended)
			{
				const G3D::Vector3& extP = GetExtendedPoint(-1, y);
				if (IsValidPoint(extP))
				{
					p[0] = extP.y;
				}
				else
				{
					p[0] = p[_irandom(1, 3)];
				}
			}
			else
			{
				p[0] = p[_irandom(1, 3)];
			}
		}
		else if (x == m_nMax)
		{
			p[0] = GetAtXY(x - size, y);// m_vHeightMap[x - size + m_nSize * y];
			p[1] = GetAtXY(x, y - size);//m_vHeightMap[x + m_nSize * (y - size)];
			p[2] = p[3] = GetAtXY(x, y + size);// m_vHeightMap[x + m_nSize * (y + size)];
			if (m_bEdgeExtended)
			{
				const G3D::Vector3& extP = GetExtendedPoint(m_nMax + 1, y);
				if (IsValidPoint(extP))
				{
					p[2] = extP.y;
				}
				else
				{
					int i = _irandom(0, 2);
					if (i == 2)
					{
						i++;
					}
					p[2] = p[i];
				}
			}
			else
			{
				int i = _irandom(0, 2);
				if (i == 2)
				{
					i++;
				}
				p[2] = p[i];
			}
		}
		else if (y == 0)
		{
			p[0] = GetAtXY(x - size, 0);// m_vHeightMap[x - size];
			p[2] = GetAtXY(x + size, 0);// m_vHeightMap[x + size];
			p[3] = GetAtXY(x, size);// m_vHeightMap[x + m_nSize * size];
			if (m_bEdgeExtended)
			{
				const G3D::Vector3& extP = GetExtendedPoint(x, -1);
				if (IsValidPoint(extP))
				{
					p[1] = extP.y;
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
		else if (y == m_nMax)
		{
			p[0] = GetAtXY(x - size, y);// m_vHeightMap[x - size + m_nSize * y];
			p[1] = GetAtXY(x, y - size);//m_vHeightMap[x + m_nSize * (y - size)];
			p[2] = GetAtXY(x + size, y);//m_vHeightMap[x + size + m_nSize * y];
			if (m_bEdgeExtended)
			{
				const G3D::Vector3& extP = GetExtendedPoint(x, m_nMax + 1);
				if (IsValidPoint(extP))
				{
					p[3] = extP.y;
				}
				else
				{
					p[3] = p[_irandom(0, 2)];
				}
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
			auto itr = m_mExtendedMap.find(x + y * m_nSize);
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

	void Diamond_Square::TrySetExtendedPoint(int x, int y, int hx, int hy, float mapWidth)
	{
		//not attached yet
		if (!m_bEdgeExtended)
		{
			SetExtendedPoint(x, y, GetDistance(x, mapWidth), GetAtXY(hx, hy), GetDistance(y, mapWidth));
		}
		//if initilized already
		else if (x >= 0 && x <= m_nMax && y >= 0 && y <= m_nMax)
		{
			auto itr = m_mExtendedMap.find(x + y * m_nSize);
			if (itr == m_mExtendedMap.end())
			{
				SetExtendedPoint(x, y, GetDistance(x, mapWidth), GetAtXY(hx, hy), GetDistance(y, mapWidth));
			}
			/*else
			{
				LogFormat("extend point exist x %d,y %d,rx %d,ry %d,h %f", x, y, hx, hy, itr->second);
			}*/
		}
		//if not the up conditions and not initilized
		else
		{
			auto v = GetExtendedPoint(x, y);
			if (!IsValidPoint(v))
			{
				SetExtendedPoint(x, y, GetDistance(x, mapWidth), GetAtXY(hx, hy), GetDistance(y, mapWidth));
			}
			/*else
			{
				LogWarningFormat("extend point x %d,y %d,rx %d,ry %d,(x %f,y %f,z %f)", x, y, hx, hy, v.x, v.y, v.z);
			}*/
		}
	}

	void Diamond_Square::GenerateTerrian(float maxCoord)
	{
		if (maxCoord <= 0)
		{
			return;
		}
		//double d = maxCoord / (float)m_nMax;
		for (int y = 0; y <= m_nMax; y++)
		{
			if (y == 0)
			{
				/*SetExtendedPoint(-1, -1, -d, GetAtXY(0, 0), -d);//x = -1,
				SetExtendedPoint(m_nSize, -1, m_nSize  * d, GetAtXY(m_nMax, 0), -d);//x = m_nSize*/
				TrySetExtendedPoint(-1, -1, 0, 0, maxCoord);
				TrySetExtendedPoint(m_nSize, -1, m_nMax, 0, maxCoord);
			}
			else if (y == m_nMax)
			{
				/*SetExtendedPoint(-1, m_nSize, -d, GetAtXY(0, m_nMax), d*m_nSize);//x = -1,
				SetExtendedPoint(m_nSize, m_nSize, m_nSize  * d, GetAtXY(m_nMax, m_nMax), d*m_nSize);//x = m_nSize*/
				TrySetExtendedPoint(-1, m_nSize, 0, m_nMax, maxCoord);
				TrySetExtendedPoint(m_nSize, m_nSize, m_nMax, m_nMax, maxCoord);
			}
			/*SetExtendedPoint(-1, y, -d, GetAtXY(0, y), d*y);//x = -1,
			SetExtendedPoint(m_nSize, y, m_nSize  * d, GetAtXY(m_nMax, y), d*y);//x = m_nSize*/
			TrySetExtendedPoint(-1, y, 0, y, maxCoord);
			TrySetExtendedPoint(m_nSize, y, m_nMax, y, maxCoord);
			for (int x = 0; x <= m_nMax; x++)
			{
				float height = GetAtXY(x, y);
				TrySetExtendedPoint(x, y, x, y, maxCoord);

				if (y == 0)
				{
					//SetExtendedPoint(x, -1, x*d, height, -d);
					TrySetExtendedPoint(x, -1, x, y, maxCoord);
				}
				else if (y == m_nMax)
				{
					//SetExtendedPoint(x, m_nSize, x*d, height, m_nSize*d);
					TrySetExtendedPoint(x, m_nSize, x, y, maxCoord);
				}
			}
		}
		/*for (int i = 0;i<m_vExtendPoints.size();i++)
		{
			LogFormat("ep at %d (x %f,y %f,z %f)",i,m_vExtendPoints[i].x,m_vExtendPoints[i].y,m_vExtendPoints[i].z);
		}*/
		int vidx = 0;
		for (int y = 0; y <= m_nMax; y++)
		{
			for (int x = 0; x <= m_nMax; x++)
			{
				vidx = x + y * m_nSize;
				Vector3 p = GetExtendedPoint(x, y);
#if UNITY_CORE
				m_vVertices[vidx] = p;
#else
				m_vVertices[vidx] = p;
#endif
				pNeibor[0] = GetExtendedPoint(x - 1, y);
				pNeibor[1] = GetExtendedPoint(x, y - 1);
				pNeibor[2] = GetExtendedPoint(x + 1, y);
				pNeibor[3] = GetExtendedPoint(x, y + 1);

				_normal[0] = -unityMesh::getNormal(p - pNeibor[0], p - pNeibor[1]);
				_normal[1] = -unityMesh::getNormal(p - pNeibor[1], p - pNeibor[2]);
				_normal[2] = -unityMesh::getNormal(p - pNeibor[2], p - pNeibor[3]);
				_normal[3] = -unityMesh::getNormal(p - pNeibor[3], p - pNeibor[0]);
				_normal[4] = _normal[0] + _normal[1] + _normal[2] + _normal[3];
				/*if (_normal[4].y <=0)
				{
					LogFormat("normal caculate error at point x %d,y %d,px %d,py %d,pz %d",x,y,p.x,p.y,p.z);
					for (int i = 0; i < 4; i++)
					{
						LogFormat("GenerateTerrian pNeibor %d (x %f,y %f,z %f)", i, pNeibor[i].x, pNeibor[i].y, pNeibor[i].z);
					}
					for (int i = 0; i < 5; i++)
					{
						LogFormat("GenerateTerrian normal %d (x %f,y %f,z %f)", i, _normal[i].x, _normal[i].y, _normal[i].z);
					}
				}*/
				m_vNormals[vidx] = unityMesh::normalize(_normal[4]);
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
				Vector3 p = GetExtendedPoint(x, y);
				pNeibor[0] = GetExtendedPoint(x - 1, y);
				pNeibor[1] = GetExtendedPoint(x, y - 1);
				pNeibor[2] = GetExtendedPoint(x + 1, y);
				pNeibor[3] = GetExtendedPoint(x, y + 1);

				_normal[0] = -unityMesh::getNormal(p - pNeibor[0], p - pNeibor[1]);
				_normal[1] = -unityMesh::getNormal(p - pNeibor[1], p - pNeibor[2]);
				_normal[2] = -unityMesh::getNormal(p - pNeibor[2], p - pNeibor[3]);
				_normal[3] = -unityMesh::getNormal(p - pNeibor[3], p - pNeibor[0]);
				_normal[4] = _normal[0] + _normal[1] + _normal[2] + _normal[3];
				/*for (int i = 0; i < 5; i++)
				{
					LogFormat("RecaculateNormal normal %d (x %f,y %f,z %f)", i, _normal[i].x, _normal[i].y, _normal[i].z);
				}*/
				pN[indexSize] = unityMesh::normalize(_normal[4]);
			}
		}
		//LogWarningFormat("at mesh %d,recaculate normal size %d,indexSize %d", mesh, size, indexSize);
	}
	void Diamond_Square::SetVerticesAndNormal(G3D::Vector3 * pV, G3D::Vector3 * pN, int32_t size, int32_t mesh)
	{
		int vidx = 0;
		if (mesh < 0 || mesh >= m_vVerticesSize.size())
		{
			LogErrorFormat("Mesh index error!%d", mesh);
		}
		int meshCount = GetMeshTheoreticalCount();
		int vertexPerMesh = m_nMax / meshCount;
		int upBound = (mesh + 1)*vertexPerMesh;
		if (upBound > m_nMax)
		{
			upBound = m_nMax;
		}
		int start = mesh * vertexPerMesh;
		//LogFormat("SetVerticesAndNormal mesh count %d,meshidx  %d,total mesh %d ,upBound %d ,start %d", meshCount, mesh, m_vVerticesSize[mesh], upBound, start);
		for (int y = start; y <= upBound; y++)
		{
			for (int x = 0; x <= m_nMax; x++)
			{
				if (vidx >= size)
				{
					LogErrorFormat("vertice size is full!%d-max size %d", vidx, size);
					return;
				}
				pV[vidx] = GetRealVertice(x, y);
				pN[vidx] = GetRealNormal(x, y);
				//LogFormat("vertice %d,%d -%d :(x %f,y %f,z %f)", x, y, vidx, pV[vidx].x, pV[vidx].y, pV[vidx].z);
				vidx++;
			}
		}
	}
}