#ifndef _UNITY_MESH_H
#define _UNITY_MESH_H

#include "Delaunay.h"
#include <vector>
#include "G3D/Vector3.h"
NS_UMESH_START
	/**

	*/
	void trianglate(const std::vector<G3D::Vector3>& input, std::vector<int32_t>& out);

NS_UMESH_END
#endif // _UNITY_MESH_E
