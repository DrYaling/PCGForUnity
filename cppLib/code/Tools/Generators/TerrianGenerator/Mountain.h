#ifndef _MOUNTAIN_H
#define _MOUNTAIN_H
#include "Generators/generator.h"
#include <stack>
using namespace G3D;

NS_GNRT_START

/*
variables:S T R C
constants:[]+-<>^   +为顺时针15°，-为逆时针15°，<为逆时针30°，>为顺时针30°，^为概率事件
axiom: S
rules:(T->^[<C+SRTSR-S>T]^S^[RT]^>S<C[+T^C]^[-R]),
（C->^S^T^C^R）
（S->[S^T>SR<S-TR+S]^[+STRS>S<T-S][-SST<SR>T+RS]^[>RSTS-SR+T>S]^[<STRS+S-T<SR])

不支持 ^[^S]这样的结构，因为^[S]和[^S]都是根据S来判断

*/
#define NORMAL_STEP 10*100 //meter

#define ROCK_STEP _frandom(100,1200)
//Lsystem_MountainGen_Point
struct L_M_Point {
public:
	char z;
	Vector3 v;
	int idx;
	explicit L_M_Point(char _z, int _index, Vector3&& _v)
	{
		z = _z;
		v = _v;
		idx = _index;
	}
};
struct MemoryPoint {
public:
	explicit MemoryPoint(Vector3&& _v, char _c, float _r, int _index)
	{
		v = _v;
		c = _c;
		r = _r;
		i = _index;
	}
	bool operator == (const MemoryPoint& right)
	{
		return c == right.c && i == right.i&& std::abs(r - right.r) < 0.00000001F && v == right.v;
	}
	char c;//操作符
	float r;//方向
	int i;
	Vector3 v;
};
static MemoryPoint mp_zero = MemoryPoint(Vector3(), 0, 0, 0);
class MountainGen:public TerrianGenerator
{
public:
	MountainGen() {}
	MountainGen(Vector3&& p, int depth);
	virtual ~MountainGen();
	void Init(int seed, int step, int32_t maxHeight, bool usePerlin = false);
	void Start(std::vector<int32_t>& triangles, std::vector<Vector3>& v3);
private:
	bool Gen();
	//生成下一个Block
	size_t GenNextBlock(char prev, size_t& rpos);
	//生成下一个var
	void GenNext(char v, size_t& rpos);
	void GenVar(char v, size_t rpos);
	//生成岩石
	void GenRock();
	inline bool CheckRP(size_t rpos)
	{
		return rpos < _tmpList.size();
	}
	inline char NextNoStep(size_t rpos)
	{
		if (CheckRP(rpos))
		{
			return _tmpList[rpos];
		}
		return 0;
	}
	inline char Next(size_t& rpos)
	{
		if (CheckRP(rpos))
		{
			return _tmpList[rpos++];
		}
		return 0;
	}
	std::vector<L_M_Point> _mother;
	std::vector<L_M_Point> _child;
	std::vector<char> _tmpList;
	std::map<char, const char*> _rules;
	int _genDepth;
	int _step;
	int _maxHeight;
	int _c;
	int _t;
	int _s;
	int _minc;
	int _mint;
	float _currentDir;
	bool _usePerlin;
	Vector3 _currentPoint;
	std::stack<MemoryPoint> _M;
};

NS_GNRT_END

#endif
