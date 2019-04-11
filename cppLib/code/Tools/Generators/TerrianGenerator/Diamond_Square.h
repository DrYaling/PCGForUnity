#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H
#include "../generator.h"
#include <vector>
namespace generator
{
	/************************************************************************/
	/*							菱形-正方形生成地形                         */
	/*				点的序号为 x=0-x = max 为0-max,y轴向上递增				*/
	/************************************************************************/
	class Diamond_Square :public TerrianGenerator
	{
	public:
		Diamond_Square(int32_t seed, int32_t I, float H, std::vector<float>& heightMap);
		virtual ~Diamond_Square();
		void SetProcessHandler(std::function<void(int32_t)> handler) { m_cbProcessHandler = handler; }
		//平滑边沿，使之可以和其他地图拼接
		inline void SetPulse(int32_t x, int32_t y, const G3D::Vector3& point, bool insertMap)
		{
			m_bEdgeExtended = true;
			SetExtendedPoint(x, y, point);
			if (insertMap && x >= 0 && x < m_nSize && y >= 0 && y < m_nSize)
			{
				int key = x + y * m_nSize;
				m_mExtendedMap.insert(std::make_pair(key, point.y));
			}
		}
		inline void SetPulse(int32_t x, int32_t y, float fx, float fy, float fz, bool insertMap)
		{
			m_bEdgeExtended = true;
			SetExtendedPoint(x, y, fx, fy, fz);
			if (insertMap && x >= 0 && x < m_nSize && y >= 0 && y < m_nSize)
			{
				int key = x + y * m_nSize;
				m_mExtendedMap.insert(std::make_pair(key, fy));
			}
		}
		void Start(const float* corner, const int32_t size = 4, std::function<void(void)> cb = nullptr);
		//根据传入的最大坐标maxCoord计算地图
		void GenerateTerrian(float maxCoord);
		//只计算需要重新计算的边沿，以和旁边的地图契合
		void RecaculateNormal(G3D::Vector3* pN, int32_t size, int32_t mesh, int32_t position);
		bool IsFinished() { return m_bIsFinished; }
		int32_t GetSquareSize() { return m_nSize; }
		void SetVerticesAndNormal(G3D::Vector3* pV, G3D::Vector3* pN, int32_t size, int32_t mesh);
		int32_t GetVerticesSize(int mesh) {
			if (mesh <0 || mesh > GetMeshTheoreticalCount())
			{
				return 0;
			}
			return m_vVerticesSize[mesh];
		}
		//mesh 理论值
		size_t GetMeshTheoreticalCount() {
			if (GetSize() % MAX_MESH_VERTICES == 0)
			{
				return GetSize() / MAX_MESH_VERTICES;
			}
			else
			{
				return GetSize() / MAX_MESH_VERTICES + 1;
			}
		}
		//因为重叠边界导致的实际mesh数量可能会比理论值大
		size_t GetMeshRealCount()
		{
			return m_vVerticesSize.size();
		}
		void ReleaseUnusedBuffer();
	private:
		void WorkThread(std::function<void(void)> cb);
		inline void Diamond(int x, int y, int size, float h);
		inline void Square(int x, int y, int size, float h);
		inline float Randomize(float h);
		inline bool IsEdge(int x, int y) { return x == 0 || y == 0 || x == m_nI || y == m_nI; }
		//防止浮点数的累计误差
		inline float GetDistance(int xy, float maxDistance)
		{
			return xy * maxDistance / (float)m_nMax;
		}
		inline void SetAtXY(int32_t x, int32_t y, float val) { m_vHeightMap[x + y * m_nSize] = val; }
		inline float GetAtXY(int x, int y) { return m_vHeightMap[x + y * m_nSize]; }
		float GetExtendHeight(int x, int y)
		{
			if (x < 0)
			{
				x = 0;
			}
			else if (x > m_nMax)
			{
				x = m_nMax;
			}
			if (y < 0)
			{
				y = 0;
			}
			else if (y > m_nMax)
			{
				y = m_nMax;
			}
			return GetAtXY(x, y);
		}
		inline size_t GetSize() { return m_vHeightMap.size(); }
		inline void	 SetExtendedPoint(int x, int y, float fx, float fy, float fz)/*x y from -1~m_nSize+1*/
		{
			m_vExtendPoints[x + 1 + (y + 1) * (m_nSize + 2)] = G3D::Vector3(fx, fy, fz);
		}
		void TrySetExtendedPoint(int x, int y, int hx, int hy, float deltaSize);
		inline void	 SetExtendedPoint(int x, int y, const G3D::Vector3& v)/*x y from -1~m_nSize+1*/
		{
			m_vExtendPoints[x + 1 + (y + 1) * (m_nSize + 2)] = v;
		}
		inline const G3D::Vector3& GetExtendedPoint(int x, int y) const/*x y from -1~m_nSize+1*/
		{
			return m_vExtendPoints[x + 1 + (y + 1) * (m_nSize + 2)];
		}
		inline const G3D::Vector3& GetRealVertice(int x, int y)
		{
			return m_vVertices[x + y * m_nSize];
		}
		inline const G3D::Vector3& GetRealNormal(int x, int y)
		{
			return m_vNormals[x + y * m_nSize];
		}
		bool IsValidPoint(const G3D::Vector3& v)
		{
			return fabsf(v.x) > 0.0001f && fabsf(v.y) > 0.0001f && fabsf(v.z) > 0.0001f;
		}
	private:
		std::function<void(int32_t)> m_cbProcessHandler;
		std::vector<float>& m_vHeightMap;
		std::vector<G3D::Vector3> m_vExtendPoints;/*x = -1,y = -1,x = m_nSize,y = m_nSize*/
		std::vector<G3D::Vector3> m_vVertices;
		std::vector<G3D::Vector3> m_vNormals;
		std::vector<int32_t> m_vVerticesSize;
		std::map<int32_t, float> m_mExtendedMap;
		float m_aPointBuffer[5];
		int32_t m_nSize;//总数 2^(2*i)+1
		float m_nH;//粗糙度
		int32_t	m_nI;//级数
		int32_t m_nMax;
		bool m_bIsFinished;
		bool m_bEdgeExtended;
	};

}
#endif
