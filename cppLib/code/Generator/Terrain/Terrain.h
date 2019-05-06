#ifndef TERRIAN_MESH_H
#define TERRIAN_MESH_H
#include "generator.h"
#include "Generators/TerrainGenerator/Diamond_Square.h"
#include "Painter/AutomaticPainter.h"
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
	float GetHeight(int32_t x, int32_t y)
	{
		generator_clamp(x, 0, m_nSize - 1);
		generator_clamp(y, 0, m_nSize - 1);
		return  m_aHeightMap[GetHeightMapIndex(x, y)];
	}
	int32_t GetSplatCount() { return m_nSplatCount; }
	int32_t GetSplatWidth() { return m_nSplatWidth; }
	uint32_t GetNeighbor(NeighborType neighbor);
	uint32_t GetI() { return m_nI; }
	bool IsWorldMap() { return m_nInstanceId == 0xffffffff; }
	uint32_t GetRealSize()
	{
		return m_nRealWidth;
	}
	bool IsInitilized()
	{
		return m_bHeightMapInitilized;
	}
	bool IsValidWorldMap()
	{
		return IsWorldMap() && m_nSize > 4;
	}
	uint32_t GetHeightMapSize() 
	{
		return m_nSize;
	}
private:
	void Init(float* heightMap, int32_t heightMapSize, float* splatMap, int32_t splatSize, int32_t splatCount);
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
	float* m_aSplatMap;
	int32_t m_nSize;
	uint32_t m_nInstanceId;
	int32_t m_nSplatCount;
	int32_t m_nSplatWidth;
	std::atomic_bool m_bHeightMapInitilized;
};
NS_GNRT_END
#endif
