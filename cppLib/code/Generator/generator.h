#ifndef GENERATOR_H
#define GENERATOR_H
#include <thread>
#include <stdlib.h>
#include <vector>
#include <map>
#include <mutex>
#include <stdio.h>
#include "G3D/Vector3.h"
#include "define.h"
#define NS_GNRT_START namespace generator{\

#define NS_GNRT_END }\

#define  UNITY_CORE 1

static inline int _irandom(int min, int max)
{
	if (min >= max)
		return min;
	return std::rand() % (max - min) + min;
}
static inline float _frandom(int min, int max)
{
	if (min >= max)
		return (float)min;
	return (float)(std::rand() % (max - min) + min) + (std::rand() % 10000) / 10000.0f;
}
static inline float _frandom_f(float min, float max)
{
	if (min >= max)
	{
		return min;
	}
	int i(min * 10000.f), a(max * 10000.f);
	return (float)(_irandom(i, a) / 10000.0f);
}
static inline bool GetChance(float chance)
{
	return _frandom_f(0, 1.0f) <= chance;
}
static inline void setRandomSeed(int seed)
{
	std::srand(seed);
}
//地形
enum class eGeographyType :int16_t
{
	e_geo_trench = 0x0000,
	e_geo_deepSea = 0x0001,
	e_geo_shallowSea = 0x0002,
	e_geo_sea_land = 0x0004,
	e_geo_flatlands = 0x0008,
	e_geo_basin = 0x0010,
	e_geo_foothill = 0x0020,
	e_geo_montains = 0x0040,
	e_geo_high_montains = 0x0080,
	e_geo_heightLand = 0x0100,
};
//sub geography property
enum class eGeographySubType :int16_t
{
	eg_sub_volcano = 1,//火山
	eg_sub_marsh = 2,//沼泽
	eg_sub_forest = 3,//森林
	eg_sub_desert = 4,//沙漠
	eg_sub_grassland = 5,//草原
	eg_sub_lake = 6,//湖泊
};
//地理属性
typedef struct geographyProperties
{
	uint64_t gType : 2;//eGeographyType
	uint64_t gSubType : 2;//eGeographySubType
	uint64_t e : 8;

}*pGeographyProperties;
#if 0
[StructLayout(LayoutKind.Sequential)]
class TerrianData
{
	private bool _useUV;
	private int _meshCount;
	private int _lodCount;
	Vector3[][] _vertices;
	Vector3[][] _normals;
	Vector2[][][] _uvs;
	int[][][] _triangles;
}
#endif
struct triangleSize_t
{
	int32_t size;
	int32_t lowBound;
	int32_t highBound;
	int32_t unit;
	int32_t extUnitX;//多余的，比如 xsize = 17 unit =3，5个unit之后余2，这个extUnit =2
	int32_t extUnitY;
};
class TerrianDataBinding
{
public:
	bool isReadable;
	bool useUv;
	int32_t meshCount;
	int32_t theoreticalMeshCount;
	int32_t lodCount;
	int32_t currentLod;
	std::vector<triangleSize_t> triangleSize;/*size == meshCount*/
	TerrianDataBinding(int32_t maxLodSize, bool _useUv) :
		useUv(_useUv),
		lodCount(maxLodSize),
		currentLod(0)
	{

	}
	void SetMeshCount(int32_t realCount, int32_t theoreticalCount)
	{
		meshCount = realCount;
		theoreticalMeshCount = theoreticalCount;
		triangleSize.resize(realCount);
	}
};
class TerrianGenerator
{
public:
	TerrianGenerator() {};
	virtual ~TerrianGenerator() {};

private:

};
#define MAX_MESH_COUNT 36
#define  MAX_MESH_VERTICES 65000
typedef int32_t*(__stdcall * MeshInitilizerCallBack)(int32_t target, int32_t type, int32_t mesh, int32_t lod, int32_t size);
typedef void(__stdcall * GeneratorNotifier)(int32_t target, int32_t type, int32_t arg0, int32_t arg1);
#define meshTopologyVertice 0
#define meshTopologyTriangle 1
#define meshTopologyUV 2
#define meshTopologyMeshCount 3
#define meshTopologyNormal 4

#define mesh_arg_seed 0
#define mesh_arg_lod 1
#define mesh_arg_I 2
#define mesh_arg_H 3
#define mesh_arg_mapWidth 4
#define mesh_arg_h0 5
#define mesh_arg_h1 6
#define mesh_arg_h2 7
#define mesh_arg_h3 8
#define mesh_arg_h4 9
#define mesh_arg_useuv 10


enum class NeighborType
{
	neighborPositionLeft = 0,
	neighborPositionBottom,
	neighborPositionRight,
	neighborPositionTop,
	neighborPositionAll
};
enum class TerrainInitType
{
	HeightMap = 0,
	AlphaMap,
};
enum class TerrainSmoothType :int32_t
{
	Smooth = 100,//平坦地形
	Rough = 250,//粗糙地形
	Sharp//尖锐地形
};
#define TERRAIN_GENERATE_VERTICES 0
#define  MAX_MAP_HEIGHT 1000.0f
#define TERRAIN_HIGH_NO_GRASS_HEIGHT 550.0f
#define TERRAIN_LOW_NO_GRASS_HEIGHT 50.0f
#define GetHeightMapIndex(x,y) x+y*m_nSize
#define GetSplatMapIndex(x,y,z,size,sizeAlpha) ((x * size + y) * sizeAlpha+z) 
#define generator_clamp(x,low,high) if (x < low) \
		x = low; \
	else if (x > high)\
		x = high;
namespace generator
{
	template <typename T>
	static inline T clamp(T a, T l, T h)
	{
		if (a < l)
			return l;
		else if (a > h)
			return  h;
		else
			return a;
	}
	static inline float GetDistance2D(float x, float y, float targetX, float targetY)
	{
		float dx = targetX - x;
		float dy = targetY - y;
		return std::sqrt(dx*dx + dy * dy);
	}
	struct  MapGeneratorData
	{
		int32_t seed;
		int32_t H;
		int32_t I;
		uint32_t worldMapSize;//map count at each world map edge
		uint32_t singleMapSize;
		uint32_t splatWidth;
		uint32_t splatCount;
		//first terrain param
		int32_t height0;
		int32_t height1;
		int32_t height2;
		int32_t height3;
		int32_t flags;// bit 0 & 1 - use native thread (0-native thread(c++),1-c# thread,2-c# main thread(single thread),3-not supported,bit 2 -use berlin noise

	};
}


#endif