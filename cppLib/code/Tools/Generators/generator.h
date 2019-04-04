#ifndef GENERATOR_H
#define GENERATOR_H
#include <thread>
#include <stdlib.h>
#include <vector>
#include <map>
#include <mutex>
#include <stdio.h>
#include "G3D/Vector3.h"
#define NS_GNRT_START namespace generator\
						{

#define NS_GNRT_END \
}

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
		return min;
	return (float)(std::rand() % (max - min) + min) + (std::rand() % 10000) / 10000.0f;
}
static inline float _frandom_f(float min, float max)
{
	if (min	>= max)
	{
		return min;
	}
	int i(min * 10000), a(max * 10000);
	return (float)(_irandom(i, a) / 10000.0f);
}
static inline void setRandomSeed(int seed)
{
	std::srand(seed);
}
//地形
enum eGeographyType :int16_t
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
enum eGeographySubType :int16_t
{
	eg_sub_volcano = 1,//火山
	eg_sub_marsh = 2,//沼泽
	eg_sub_forest= 3,//森林
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
struct TerrianGeneratorData
{
	G3D::Vector3 pos;
	int32_t seed;
	bool optimaze;
};
class TerrianGenerator
{
public:
	TerrianGenerator() {};
	virtual ~TerrianGenerator() {};

private:

};
#define MAX_MESH_COUNT 36
// typedef geography* pGeography;
#endif