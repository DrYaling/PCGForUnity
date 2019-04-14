#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H
#include "../generator.h"
#include <vector>
#include <algorithm>
namespace generator
{
#define BLUR_SIZE 2
	//typedef std::function<bool(int32_t /*x*/, int32_t /*y*/, NeighborType /*neighbor*/, float& /*h*/, void*/*owner*/)> GetNeighborVertice;
	typedef bool(__fastcall *GetNeighborVertice)(int32_t /*x*/, int32_t /*y*/, NeighborType /*neighbor*/, float& /*h*/, void*/*owner*/);
	/************************************************************************/
	/*							菱形-正方形生成地形                         */
	/*				点的序号为 x=0-x = max 为0-max,y轴向上递增				*/
	/************************************************************************/
	class Diamond_Square :public TerrianGenerator
	{
	public:
		Diamond_Square(int32_t seed, int32_t I, float H, /*std::vector<float>&*/float* heightMap, void* owner);
		virtual ~Diamond_Square();
		void SetProcessHandler(std::function<void(int32_t)> handler) { m_cbProcessHandler = handler; }
		//平滑边沿，使之可以和其他地图拼接
		inline void SetPulse(int32_t x, int32_t y, const G3D::Vector3& point, bool insertMap)
		{
			m_bEdgeExtended = true;
			//SetExtendedPoint(x, y, point);
			if (insertMap && x >= 0 && x < m_nSize && y >= 0 && y < m_nSize)
			{
				int key = GetHeightMapIndex(x, y);
				m_mExtendedMap.insert(std::make_pair(key, point.y));
			}
		}
		inline void SetPulse(int32_t x, int32_t y, float fx, float fy, float fz, bool insertMap)
		{
			m_bEdgeExtended = true;
			//SetExtendedPoint(x, y, fx, fy, fz);
			if (insertMap && x >= 0 && x < m_nSize && y >= 0 && y < m_nSize)
			{
				int key = GetHeightMapIndex(x, y);
				m_mExtendedMap.insert(std::make_pair(key, fy));
			}
		}
		void Start(const float* corner, const int32_t size = 4, std::function<void(void)> cb = nullptr);
		void SetGetVerticeCallBack(GetNeighborVertice cb) { m_cbGetNeighborVertice = cb; }
		inline float GetHeight(int x, int y) {
			generator_clamp(x, 0, m_nMax);
			generator_clamp(y, 0, m_nMax);
			return m_vHeightMap[GetHeightMapIndex(x, y)];
		}
		bool IsFinished() { return m_bIsFinished; }
		int32_t GetSquareSize() { return m_nSize; }
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
		size_t GetAlloc() {
			size_t t = 0;
			t += sizeof(Diamond_Square);
			return t;
		}
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
		inline void SetHeight(int32_t x, int32_t y, float val) { m_vHeightMap[GetHeightMapIndex(x, y)] = val; }
		inline size_t GetSize() { return m_nheightMapSize; }
		bool IsValidPoint(const G3D::Vector3& v)
		{
			return fabsf(v.x) > 0.0001f && fabsf(v.y) > 0.0001f && fabsf(v.z) > 0.0001f;
		}
		void Blur();
		void Smooth(int32_t x, int32_t y)
		{
			float h = 0.0F;
			h += GetHeight(x, y) ;
			h += GetHeight(x + 1, y) ;
			h += GetHeight(x - 1, y) ;
			h += GetHeight(x + 1, y + 1)  * 0.75F;
			h += GetHeight(x - 1, y + 1)  * 0.75F;
			h += GetHeight(x + 1, y - 1)  * 0.75F;
			h += GetHeight(x - 1, y - 1)  * 0.75F;
			h += GetHeight(x, y + 1) ;
			h += GetHeight(x, y - 1) ;
			h /= 8.0F;
			SetHeight(x, y, h);
		}
	private:
		void* m_pOwner;
		float* m_vHeightMap;
		int32_t m_nheightMapSize;
		GetNeighborVertice m_cbGetNeighborVertice;
		std::function<void(int32_t)> m_cbProcessHandler;
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
