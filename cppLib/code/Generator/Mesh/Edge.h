#ifndef H_EDGE
#define H_EDGE

#include "Mesh.h"
#include "Vector2Lite.h"
NS_UMESH_START
class Edge
{
public:

	std::string toString()
	{
		std::string s;
		char buff[128] = { 0 };
		sprintf_s(buff, "Edge:\n\t(%s,%s)", p1.toString().c_str(), p2.toString().c_str());
		s.append(buff);
		return s;
	}
	inline bool operator == ( const unityMesh::Edge & e2)
	{
		return 	(p1 == e2.p1 && p2 == e2.p2) ||
			(p1 == e2.p2 && p2 == e2.p1);
	}
	Edge(const Vector2 &p1, const Vector2 &p2) : p1(p1), p2(p2), isBad(false) {};
	Edge(const Edge &e) : p1(e.p1), p2(e.p2), isBad(false) {};

	Vector2 p1;
	Vector2 p2;

	bool isBad;
};

/*
inline std::ostream &operator << (std::ostream &str, unityMesh::Edge const &e)
{
	return str << "Edge " << e.p1 << ", " << e.p2;
}
*/

NS_UMESH_END
#endif 

