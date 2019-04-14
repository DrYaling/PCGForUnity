#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H
#include "../generator.h"
#include <vector>
namespace generator
{
#define BLUR_SIZE 2
#if TERRAIN_GENERATE_VERTICES
	typedef std::function<bool(int32_t /*x*/, int32_t /*y*/, int32_t /*neighbor*/, G3D::Vector3& /*p*/)> GetNeighborVertice;
#else
	typedef std::function<bool(int32_t /*x*/, int32_t /*y*/, NeighborType /*neighbor*/, float& /*h*/)> GetNeighborVertice;
#endif
	/************************************************************************/
	/*							����-���������ɵ���                         */
	/*				������Ϊ x=0-x = max Ϊ0-max,y�����ϵ���				*/
	/************************************************************************/
	class Diamond_Square :public TerrianGenerator
	{
	public:
		Diamond_Square(int32_t seed, int32_t I, float H, /*std::vector<float>&*/float* heightMap);
		virtual ~Diamond_Square();
		void SetProcessHandler(std::function<void(int32_t)> handler) { m_cbProcessHandler = handler; }
		//ƽ�����أ�ʹ֮���Ժ�������ͼƴ��
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
#if TERRAIN_GENERATE_VERTICES
		//���ݴ�����������maxCoord�����ͼ
		void GenerateTerrian(float maxCoord);
		//ֻ������Ҫ���¼���ı��أ��Ժ��Աߵĵ�ͼ����
		void RecaculateNormal(G3D::Vector3* pN, int32_t size, int32_t mesh, int32_t position);
		void SetVerticesAndNormal(G3D::Vector3* pV, G3D::Vector3* pN, int32_t size, int32_t mesh);
		inline const G3D::Vector3& GetRealVertice(int x, int y);
		inline const G3D::Vector3& GetRealNormal(int x, int y)
		{
			return m_vNormals[x + y * m_nSize];
		}
		int32_t GetVerticesSize(int mesh) {
			if (mesh <0 || mesh > GetMeshTheoreticalCount())
			{
				return 0;
			}
			return m_vVerticesSize[mesh];
		}
		//��Ϊ�ص��߽絼�µ�ʵ��mesh�������ܻ������ֵ��
		size_t GetMeshRealCount()
		{
			return m_vVerticesSize.size();
		}
#endif
		inline float GetAtXY(int x, int y) { return m_vHeightMap[GetHeightMapIndex(x, y)]; }
		bool IsFinished() { return m_bIsFinished; }
		int32_t GetSquareSize() { return m_nSize; }
		//mesh ����ֵ
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
			//t += sizeof(float)*m_nheightMapSize;
#if TERRAIN_GENERATE_VERTICES
			t += sizeof(G3D::Vector3)*m_vVertices.capacity();
			t += sizeof(G3D::Vector3)*m_vNormals.capacity();
			t += sizeof(int32_t)*m_vVerticesSize.capacity();
			t += sizeof(G3D::Vector3) * 5 + sizeof(float) * 5;
#endif
			return t;
		}
	private:
		void WorkThread(std::function<void(void)> cb);
		inline void Diamond(int x, int y, int size, float h);
		inline void Square(int x, int y, int size, float h);
		inline float Randomize(float h);
		inline bool IsEdge(int x, int y) { return x == 0 || y == 0 || x == m_nI || y == m_nI; }
		//��ֹ���������ۼ����
		inline float GetDistance(int xy, float maxDistance)
		{
			return xy * maxDistance / (float)m_nMax;
		}
		inline void SetAtXY(int32_t x, int32_t y, float val) { m_vHeightMap[GetHeightMapIndex(x,y)] = val; }
		inline size_t GetSize() { return m_nheightMapSize; }
		bool IsValidPoint(const G3D::Vector3& v)
		{
			return fabsf(v.x) > 0.0001f && fabsf(v.y) > 0.0001f && fabsf(v.z) > 0.0001f;
		}
		void Blur();
		void SetBlurAtXY(int32_t x, int32_t y, float val)
		{
			//middle 0.4
			//edge 0.5/8 = 
#if false
			static int edgeSize = BLUR_SIZE / 2;
			if (x >= edgeSize && x <= m_nMax - edgeSize && y >= edgeSize && y <= m_nMax - edgeSize)
			{
				static float edge_f = 0.4f / (float)(BLUR_SIZE*BLUR_SIZE - 1);
				val *= 0.6f;
				for (size_t iy = -edgeSize; iy <= edgeSize; iy++)
				{
					for (size_t ix = -edgeSize; ix <= edgeSize; ix++)
					{
						if (ix == 0 && iy == 0)
						{
							continue;
						}
						val += GetAtXY(x + ix, y + iy)*edge_f;
					}
				}
			}
#endif
			if (x > 0 && x < m_nMax && y >0 && y < m_nMax)
			{
				float tm = GetAtXY(x - 1, y);
				tm += GetAtXY(x, y - 1);
				tm += GetAtXY(x + 1, y);
				tm += GetAtXY(x, y + 1);
				val = val * 0.5f + tm * 0.5f / 4.0f;
			}
			SetAtXY(x, y, val);
		}
	private:
		float* m_vHeightMap;
		int32_t m_nheightMapSize;
		GetNeighborVertice m_cbGetNeighborVertice;
		std::function<void(int32_t)> m_cbProcessHandler;
		std::map<int32_t, float> m_mExtendedMap;
#if TERRAIN_GENERATE_VERTICES
		std::vector<G3D::Vector3> m_vVertices;
		std::vector<G3D::Vector3> m_vNormals;
		std::vector<int32_t> m_vVerticesSize;
		//std::vector<G3D::Vector3> m_vExtendPoints;/*x = -1,y = -1,x = m_nSize,y = m_nSize*/
		G3D::Vector3 pNeibor[5];
		G3D::Vector3 m_stNormalBuffer;
#endif
		float m_aPointBuffer[5];
		int32_t m_nSize;//���� 2^(2*i)+1
		float m_nH;//�ֲڶ�
		int32_t	m_nI;//����
		int32_t m_nMax;
		float m_fDeltaSize;//���Ӵ�С
		bool m_bIsFinished;
		bool m_bEdgeExtended;
	};

}
#endif
