#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H
#include "generator.h"
#include <vector>
#include <algorithm>
#include "Logger/Logger.h"
#include <atomic>
namespace generator
{
#define BLUR_SIZE 2
	typedef bool(__fastcall *GetNeighborHeightCallBack)(int32_t /*x*/, int32_t /*y*/, NeighborType /*neighbor*/, uint32_t/**/, float&/*height*/);
	/************************************************************************/
	/*							菱形-正方形生成地形                         */
	/*				点的序号为 x=0-x = max 为0-max,y轴向上递增				*/
	/************************************************************************/
	class Diamond_Square :public TerrianGenerator
	{
	public:
		Diamond_Square();
		virtual ~Diamond_Square();
		void Initilize(uint32_t owner, int32_t seed, int32_t I, float H, float* heightMap);
		void SetProcessHandler(std::function<void(int32_t)> handler) { m_cbProcessHandler = handler; }
		//平滑边沿，使之可以和其他地图拼接
		inline void SetPulse(int32_t x, int32_t y, float height)
		{
			if (x >= 0 && x < m_nSize && y >= 0 && y < m_nSize)
			{
				m_bEdgeExtended = true;
				int key = GetHeightMapIndex(x, y);
				m_mExtendedMap.insert(std::make_pair(key, height));
				//LogFormat("map %d extend at x %d,y %d,height %f", m_Owner, x, y, height);
			}
		}
		void Reset();
		void Start(const float* corner, const int32_t size = 4, int32_t mapWidth = 10, std::function<void(void)> cb = nullptr);
		void SetGetNeighborHeightCallBack(GetNeighborHeightCallBack cb) { m_cbGetNeighborHeight = cb; }
		inline float GetHeight(int x, int y) const
		{
			generator_clamp(x, 0, m_nMax);
			generator_clamp(y, 0, m_nMax);
			return m_vHeightMap[GetHeightMapIndex(x, y)];
		}
		inline bool GetExtendedHeight(int32_t x, int32_t y, float& height)
		{
			const std::map<int32_t, float>::iterator& itr = m_mExtendedMap.find(GetHeightMapIndex(x, y));
			if (itr != m_mExtendedMap.end())
			{
				height = itr->second;
				//LogFormat("map %d GetExtendedHeight at x %d,y %d,height is %f",m_Owner, x, y, height);
				return true;
			}
			return false;
		}
		bool IsFinished() const { return m_bIsFinished; }
		int32_t GetSquareSize() const { return m_nSize; }
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
		void ReleaseUnusedBuffer();
	private:
		void WorkThread(std::function<void(void)> cb);
		inline void Diamond(int x, int y, int size, float h);
		inline void Square(int x, int y, int size, float h);
		static inline float Randomize(float h);
		inline bool IsEdge(int x, int y) const { return x == 0 || y == 0 || x == m_nI || y == m_nI; }
		//防止浮点数的累计误差
		float GetDistance(int xy, float maxDistance) const
		{
			return xy * maxDistance / float(m_nMax);
		}

		void SetHeight(int32_t x, int32_t y, float val) const { m_vHeightMap[GetHeightMapIndex(x, y)] = val; }
		size_t GetSize() const { return m_nheightMapSize; }
		bool GetHeightOnWorldMap(int32_t x, int32_t y, NeighborType neighbor, float& p) const
		{
			if (m_bEdgeExtended && m_cbGetNeighborHeight(x, y, neighbor, m_Owner, p))
			{
				return true;
			}
			return false;
		}
		/*flush map refered to m_mExtendedMap*/
		void Flush();
		void Blur(bool perlin = false) const;
		void Smooth(int32_t x, int32_t y) const;
	private:
		uint32_t m_Owner;
		float* m_vHeightMap;
		int32_t m_nheightMapSize;
		GetNeighborHeightCallBack m_cbGetNeighborHeight;
		std::function<void(int32_t)> m_cbProcessHandler;
		std::map<int32_t, float> m_mExtendedMap;
		int32_t m_nSize;//总数 2^(2*i)+1
		float m_nH;//粗糙度
		int32_t	m_nI;//级数
		int32_t m_nMax;
		float m_fDeltaSize;
		bool m_bIsFinished;
		std::atomic_bool m_bEdgeExtended;
		bool m_bInitilized;
	};

}
#endif
