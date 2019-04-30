#ifndef H_TRIANGLE
#define H_TRIANGLE

#include "Mesh.h"
#include "edge.h"

#include <assert.h>
#include <math.h>
NS_UMESH_START
/*
hard triangle with faster speed but cost more memory
*/
class Triangle2D
{
public:


	using EdgeType = Edge;
	using VertexType = Vector2;

	inline bool operator == (const unityMesh::Triangle2D &t2)
	{
		return	(p1 == t2.p1 || p1 == t2.p2 || p1 == t2.p3) &&
			(p2 == t2.p1 || p2 == t2.p2 || p2 == t2.p3) &&
			(p3 == t2.p1 || p3 == t2.p2 || p3 == t2.p3);
	}
	Triangle2D(const VertexType &_p1, const VertexType &_p2, const VertexType &_p3)
		: p1(_p1), p2(_p2), p3(_p3),
		e1(_p1, _p2), e2(_p2, _p3), e3(_p3, _p1), isBad(false)
	{}

	bool containsVertex(const VertexType &v)
	{
		return p1 == v || p2 == v || p3 == v;
	}

	bool circumCircleContains(const VertexType &v)
	{
		float ab = (p1.x * p1.x) + (p1.y * p1.y);
		float cd = (p2.x * p2.x) + (p2.y * p2.y);
		float ef = (p3.x * p3.x) + (p3.y * p3.y);

		float circum_x = (ab * (p3.y - p2.y) + cd * (p1.y - p3.y) + ef * (p2.y - p1.y)) / (p1.x * (p3.y - p2.y) + p2.x * (p1.y - p3.y) + p3.x * (p2.y - p1.y)) / 2.f;
		float circum_y = (ab * (p3.x - p2.x) + cd * (p1.x - p3.x) + ef * (p2.x - p1.x)) / (p1.y * (p3.x - p2.x) + p2.y * (p1.x - p3.x) + p3.y * (p2.x - p1.x)) / 2.f;
		float circum_radius = sqrtf(((p1.x - circum_x) * (p1.x - circum_x)) + ((p1.y - circum_y) * (p1.y - circum_y)));

		float dist = sqrtf(((v.x - circum_x) * (v.x - circum_x)) + ((v.y - circum_y) * (v.y - circum_y)));
		return dist <= circum_radius;
	}
	std::string toString()
	{
		std::string s;
		char buff[128] = { 0 };
		sprintf_s(buff, "Triangle:\n\t(%s,%s,%s)",p1.toString().c_str() , p2.toString().c_str(),p3.toString().c_str());
		s.append(buff);
		return s;
	}

	Vector2 p1;
	Vector2 p2;
	Vector2 p3;
	EdgeType e1;
	EdgeType e2;
	EdgeType e3;
	bool isBad;
};
/*
inline std::ostream &operator << (std::ostream &str, const unityMesh::Triangle2D & t)
{
	return str << "Triangle:" << std::endl << "\t" << t.p1 << std::endl << "\t" << t.p2 << std::endl << "\t" << t.p3 << std::endl << "\t" << t.e1 << std::endl << "\t" << t.e2 << std::endl << "\t" << t.e3 << std::endl;

}*/
NS_UMESH_END


#endif
