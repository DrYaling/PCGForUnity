#include "Diamond_Square.h"
#include "Mesh/UnityMesh.h"
#include "Logger/Logger.h"
NS_GNRT_START

using namespace G3D;
Diamond_Square::Diamond_Square(int32_t seed, int32_t I, float H) :
	m_nI(I),
	m_nH(H / 100.0f),
	m_bIsFinished(false),
	m_bEdgeExtended(false)
{
	setRandomSeed(seed);
	m_nSize = std::pow(2, 2 * I) + 1;
	m_nMax = m_nSize - 1;
	m_vHeightMap.resize(m_nSize*m_nSize);
	m_vExtendPoints.resize((m_nSize + 2)*(m_nSize + 2));
}

Diamond_Square::~Diamond_Square()
{
	m_vHeightMap.clear();

}
void Diamond_Square::Start(const float * corner, const int32_t size)
{
	m_bIsFinished = false;
	if (size != 4 || nullptr == corner)
	{
		LogError("Diamond_Square Start Fail!");
		return;
	}
	int meshCount = GetSize() / 65000;
	if (meshCount > MAX_MESH_COUNT)
	{

		return;
	}
	SetAtXY(0, 0, corner[0]);
	SetAtXY(m_nMax, 0, corner[1]);
	SetAtXY(0, m_nMax, corner[2]);
	SetAtXY(m_nMax, m_nMax, corner[3]);
	//std::thread t(std::bind(&Diamond_Square::WorkThread, this));
	//t.detach();
	LogFormat("Diamond_Square Start,H %f,I %d,maxSize %d,meshCount %d", m_nH, m_nI, m_nSize, meshCount);
	WorkThread();
}

void Diamond_Square::WorkThread()
{
	float _H = m_nH;
	float process = 0;//max 100
	int Iprocess = 0;
	float processSpeedPerWhile = 1 / std::log2f(m_nMax) * 100.0f;
	LogFormat("processSpeedPerWhile %f", processSpeedPerWhile);
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
	LogFormat("ds over,total size %d,should be %d", genLen, m_nSize*m_nSize - 4);
	/*for (auto f : m_vHeightMap)
	{
		if (f > -1 && f < 1)
			printf_s("%2f ", f);
	}
	printf_s("\n");*/
	m_bIsFinished = true;
}
//菱形如果遇到边界情况，简单的从非边界点取一个来做边界数据
inline void Diamond_Square::Diamond(int x, int y, int size, float h)
{
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
			p[0] = GetExtendedPoint(-1, y).y;
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
			p[2] = GetExtendedPoint(m_nMax + 1, y).y;
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
			p[1] = GetExtendedPoint(x, -1).y;
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
			p[3] = GetExtendedPoint(x, m_nMax + 1).y;
		}
		else
		{
			int i = _irandom(0, 2);
			p[3] = p[i];
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
	//LogFormat("diamond x %d,y %d,p %f,h %f,r %f", x, y, p, h, m_vHeightMap[x + m_nSize * y]);
}
//正方形生成不用考虑边界条件
inline void Diamond_Square::Square(int x, int y, int size, float h)
{
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


void Diamond_Square::GenerateTerrian(std::vector<int32_t>* triangles, std::vector<Vector3>* v3, std::vector<G3D::Vector3>* normal, float maxCoord)
{
	if (maxCoord <= 0)
	{
		return;
	}
	if (nullptr == triangles || nullptr == v3)
	{
		return;
	}
	int meshCount = GetSize() / 65000 + 1;
	LogFormat("mesh Count %d", meshCount);
	if (meshCount > MAX_MESH_COUNT)
	{

		return;
	}
	double d = maxCoord / (float)m_nSize;
	int idx = 0;
	int	 outBoundY(m_nMax);
	int obY(m_nMax);
	outBoundY = obY = m_nMax / meshCount;
	LogFormat("outBoundY %d,d %f", outBoundY, d);
	int startY(0);
	for (int y = 0; y <= m_nMax; y++)
	{
		if (!m_bEdgeExtended)
		{
			if (y == 0)
			{
				SetExtendedPoint(-1, -1, -d, GetAtXY(0, 0), -d);//x = -1,
				SetExtendedPoint(m_nSize, -1, m_nSize  * d, GetAtXY(m_nMax, 0), -d);//x = m_nSize
			}
			else if (y == m_nMax)
			{
				SetExtendedPoint(-1, m_nSize, -d, GetAtXY(0, m_nMax), d*m_nSize);//x = -1,
				SetExtendedPoint(m_nSize, m_nSize, m_nSize  * d, GetAtXY(m_nMax, m_nMax), d*m_nSize);//x = m_nSize
			}
			SetExtendedPoint(-1, y, -d, GetAtXY(0, y), d*y);//x = -1,
			SetExtendedPoint(m_nSize, y, m_nSize  * d, GetAtXY(m_nMax, y), d*y);//x = m_nSize
		}
		for (int x = 0; x <= m_nMax; x++)
		{
			float height = GetAtXY(x, y);
			SetExtendedPoint(x, y, x*d, height, y*d);
			if (!m_bEdgeExtended)//添加上下边沿
			{
				if (y == 0)
				{
					SetExtendedPoint(x, -1, x*d, height, -d);
				}
				else if (y == m_nMax)
				{
					SetExtendedPoint(x, m_nSize, x*d, height, m_nSize*d);
				}
			}
		}
	}
	Vector3 pNeibor[4];
	Vector3 _normal[4];
	while (idx < meshCount)
	{
		v3[idx].resize(m_nSize*(outBoundY - startY + 1));
		normal[idx].resize(v3[idx].size());
		int vidx = 0;
		for (int y = startY; y <= m_nMax; y++)
		{
			for (int x = 0; x <= m_nMax; x++)
			{
				Vector3 p = GetExtendedPoint(x, y);
#if UNITY_CORE
				v3[idx][vidx] = GetExtendedPoint(x, y);
#else
				v3[idx][vidx] = GetExtendedPoint(x, y);
#endif
				pNeibor[0] = GetExtendedPoint(x - 1, y);
				pNeibor[1] = GetExtendedPoint(x, y - 1);
				pNeibor[2] = GetExtendedPoint(x + 1, y);
				pNeibor[3] = GetExtendedPoint(x, y + 1);

				_normal[0] = -unityMesh::getNormal(p - pNeibor[0], p - pNeibor[1]);
				_normal[1] = -unityMesh::getNormal(p - pNeibor[1], p - pNeibor[2]);
				_normal[2] = -unityMesh::getNormal(p - pNeibor[2], p - pNeibor[3]);
				_normal[3] = -unityMesh::getNormal(p - pNeibor[3], p - pNeibor[0]);
				_normal[0] = _normal[0] + _normal[1] + _normal[2] + _normal[3];
				normal[idx][vidx] = unityMesh::normalize(_normal[0]);
				vidx++;
			}
			if (y >= outBoundY)
			{
				break;
			}
		}
		for (int ti = 0, vi = 0, y = startY; y < outBoundY && y < m_nMax; y++, vi++)
		{
			for (int x = 0; x < m_nMax; x++, ti += 6, vi++)
			{
				int p0 = vi;
				int p1 = vi + 1;
				int p2 = vi + m_nMax + 1;
				int p3 = vi + m_nMax + 2;
				triangles[idx].push_back(p0);
				triangles[idx].push_back(p2);
				triangles[idx].push_back(p1);
				triangles[idx].push_back(p1);
				triangles[idx].push_back(p2);
				triangles[idx].push_back(p3);
			}
		}
		//LogFormat("mesh %d v size %d,triangle size %d", idx, v3[idx].size(), triangles[idx].size());
		startY = outBoundY;//因为最上面和最右边一排不计算三角形，所以在交界处需要多计算一次
		outBoundY += obY;
		idx++;
	}
}
void Diamond_Square::AddEdge(const Vector3 * edge, int32_t size, int32_t edgeType)
{
	if (!edge)
	{
		LogErrorFormat("Add Edge With Empty data");
		return;
	}
	if (size != m_nSize + 1)
	{
		LogErrorFormat("Add Edge With Wrong size!");
		return;
	}
	if (edgeType == 0)//y = -1
	{
		for (int x = -1; x <= m_nMax; x++)
		{
			const G3D::Vector3& p = edge[x + 1];
			SetExtendedPoint(x, -1, p.x, p.y, p.z);
		}
	}
	else if (edgeType == 1) //x = m_nMax,
	{
		for (int y = -1; y <= m_nMax; y++)
		{
			const G3D::Vector3& p = edge[y + 1];
			SetExtendedPoint(m_nMax, y, p.x, p.y, p.z);
		}
	}
	else if (edgeType == 2)//y = m_nMax
	{
		for (int x = -1; x <= m_nMax; x++)
		{
			const G3D::Vector3& p = edge[x + 1];
			SetExtendedPoint(x, m_nMax, p.x, p.y, p.z);
		}
	}
	else//x = -1
	{
		for (int y = -1; y <= m_nMax; y++)
		{
			const G3D::Vector3& p = edge[y + 1];
			SetExtendedPoint(-1, y, p.x, p.y, p.z);
		}
	}
	m_bEdgeExtended = true;
}

NS_GNRT_END