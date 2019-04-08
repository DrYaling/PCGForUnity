#ifndef _UNITY_MESH_H
#define _UNITY_MESH_H

#include "Delaunay.h"
#include <vector>
#include "G3D/Vector3.h"
NS_UMESH_START
	/**

	*/
	void trianglate(const std::vector<G3D::Vector3>& input, std::vector<int32_t>& out);

G3D::Vector3 getNormal(const G3D::Vector3& p0, const G3D::Vector3& p1)
{
	//a X b = (l,m,n) x (o,p,q) = (mq-np,no-lq,lp-mo)
	return G3D::Vector3(p0.y*p1.z - p0.z*p1.y, p0.z*p1.x - p0.x*p1.z, p0.x*p1.y - p0.y*p1.x);
}
G3D::Vector3 normalize(G3D::Vector3& p)
{
	return p / p.length();
}
NS_UMESH_END
#endif // _UNITY_MESH_E
