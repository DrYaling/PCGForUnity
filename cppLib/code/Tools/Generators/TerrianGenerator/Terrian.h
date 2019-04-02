/*
created by zxb @2018-5-4
地形生成器
*/

#ifndef _TERRIAN_H
#define _TERRIAN_H
#include "../generator.h"
using namespace G3D;

NS_GNRT_START

struct TerrianPoint
{
public:
	Vector3 point;
	char k;
	geographyProperties gp;
};
//粗略地形生成器
class GiantTerrianGen 
{
private:
	std::vector<TerrianPoint> _terrian;
};
//地形生成器
class TerrianGen 
{
public:
	bool Init(int32_t seed);
protected:
	//生成高度图
	void GenHeightMap();
private:
	//ds分形计算
	void ExcedDiamondSquare();
	std::vector<TerrianPoint> _terrian;
};
NS_GNRT_END
#endif
