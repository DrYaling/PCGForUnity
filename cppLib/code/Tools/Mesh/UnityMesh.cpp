#include "UnityMesh.h"
#include "Generators/generator.h"
NS_UMESH_START

void trianglate(const std::vector<G3D::Vector3>& input,std::vector<int32_t>& out)
{
	std::vector<Vector2> v2;
	int index = 0;
	for (auto v3 : input)
	{
#if UNITY_CORE
		auto v = Vector2(v3.x, v3.z, index++);
#else
		auto v = Vector2(v3.x, v3.y, index++);
#endif
		v2.push_back(v);
	}
	Delaunay triangulation;
	std::vector<Triangle2D> triangles = triangulation.triangulate(v2);
	for (auto t : triangles)
	{
		out.push_back(t.p1.index);
		out.push_back(t.p2.index);
		out.push_back(t.p3.index);
		//LogFormat("trianglate 1", "%d,%d,%d", t.p1.index, t.p2.index,t.p3.index);
	}
}

NS_UMESH_END