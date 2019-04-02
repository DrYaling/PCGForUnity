#ifndef H_VECTOR2_H
#define H_VECTOR2_H
#include "Mesh.h"
#include <iostream>
#include <cmath>
NS_UMESH_START
class Vector2
{
public:
	//
	// Constructors
	//

	bool operator==(const Vector2& other) const
	{
		return (x == other.x) && (y == other.y);
	}
	Vector2()
	{
		x = 0;
		y = 0;
	}

	Vector2(float _x, float _y)
	{
		x = _x;
		y = _y;
	}
	Vector2(float _x, float _y,int32_t idx)
	{
		x = _x;
		y = _y;
		index = idx;
	}

	Vector2(const Vector2 &v)
	{
		x = v.x;
		y = v.y;
		index = v.index;
	}

	void set(const Vector2 &v)
	{
		x = v.x;
		y = v.y;
		index = v.index;
	}

	//
	// Operations
	//	
	float dist2(const Vector2 &v)
	{
		float dx = x - v.x;
		float dy = y - v.y;
		return dx * dx + dy * dy;
	}

	float dist(const Vector2 &v)
	{
		return sqrtf(dist2(v));
	}
	std::string toString()
	{
		std::string s;
		char buff[32] = { 0 };
		sprintf_s(buff, "(%.2f,%.2f)", x, y);
		s.append(buff);
		return s;
	}
	float x;
	float y;
	int32_t index;

};
/*
std::ostream &operator << (std::ostream &str, const unityMesh::Vector2& v)
{
	return str << "Point x: " << v.x << " y: " << v.y;
}*/
/*

bool operator == (const unityMesh::Vector2& v1,const  unityMesh::Vector2& v2)
{
	return (v1.x == v2.x) && (v1.y == v2.y);
}*/

NS_UMESH_END

#endif
