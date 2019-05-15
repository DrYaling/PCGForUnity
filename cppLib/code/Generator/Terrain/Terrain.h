#ifndef TERRIAN_MESH_H
#define TERRIAN_MESH_H
#include "generator.h"
#include <memory>
#include <atomic>
NS_GNRT_START
class MapGenerator;
class Terrain :public std::enable_shared_from_this<Terrain>
{
	friend class MapGenerator;
public:
	Terrain() = delete;
	Terrain(uint32_t ins, uint32_t I, uint32_t heightMapSize);
	Terrain(const Terrain& t) = delete;
	Terrain(const Terrain&& t) = delete;
	~Terrain();
	void InitNeighbor(NeighborType edge, std::shared_ptr<Terrain> mesh);
	float GetHeight(int32_t x, int32_t y) const
	{
		generator_clamp(x, 0, m_nSize - 1);
		generator_clamp(y, 0, m_nSize - 1);
		return  m_aHeightMap[GetHeightMapIndex(x, y)];
	}
	uint32_t GetNeighbor(NeighborType neighbor) const;
	uint32_t GetI() const { return m_nI; }
	bool IsWorldMap() const { return m_nInstanceId == 0xffffffff; }
	uint32_t GetRealSize() const
	{
		return m_nRealWidth;
	}
	bool IsInitilized() const
	{
		return m_bHeightMapInitilized;
	}
	bool IsValidWorldMap() const
	{
		return IsWorldMap() && m_nSize > 4;
	}
	uint32_t GetHeightMapSize() const
	{
		return m_nSize;
	}
private:
	void Init(float* heightMap, int32_t heightMapSize);
	bool GetNeighborHeight(int32_t x, int32_t y, NeighborType neighbor, float & p);
private:
	std::shared_ptr<Terrain> m_pLeftNeighbor;
	std::shared_ptr<Terrain> m_pRightNeighbor;
	std::shared_ptr<Terrain> m_pBottomNeighbor;
	std::shared_ptr<Terrain> m_pTopNeighbor;
	G3D::Vector3 m_Position;
	uint32_t m_nI;
	uint32_t m_nRealWidth;
	float* m_aHeightMap;
	int32_t m_nSize;
	uint32_t m_nInstanceId;
	std::atomic_bool m_bHeightMapInitilized;
};
NS_GNRT_END
#endif
