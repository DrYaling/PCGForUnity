/*
created by zxb @2018-5-4
����������
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
//���Ե���������
class GiantTerrianGen 
{
private:
	std::vector<TerrianPoint> _terrian;
};
//����������
class TerrianGen 
{
public:
	bool Init(int32_t seed);
protected:
	//���ɸ߶�ͼ
	void GenHeightMap();
private:
	//ds���μ���
	void ExcedDiamondSquare();
	std::vector<TerrianPoint> _terrian;
};
NS_GNRT_END
#endif
