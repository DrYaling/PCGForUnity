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

// typedef geography* pGeography;
#endif