#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H
#include "G3D/Vector3.h"
#include "generator.h"
using namespace G3D;
NS_GNRT_START
/*
void InitGenerator(int32_t seed, const int32_t* args, int32_t argSize, bool optimalize);
void GenMeshData(int32_t type,int32_t* vSize);
void GetGeneratorData(Vector3* pV, int32_t* pI, int arg0);
void GetGeneratorTrianglesData(int32_t* pI, int arg0);
int32_t* GetGeneratorTrianglesData(int arg0);
int32_t GetGeneratorVerticesData(Vector3* pV,Vector3* pN, int index);
*/

void Internal_InitTerrianMesh(int32_t instanceId,int32_t* args,int32_t argsize, MeshInitilizerCallBack cb, GeneratorNotifier notifier);
void Internal_StartGenerateOrLoad(int32_t instanceId);
void Internal_ResetLod(int32_t instanceId,int32_t lod);
void Internal_RegisterTerrianMeshBinding(int32_t instance);
void Internal_ReleaseGenerator(int32_t instance);
void Internal_FlushMeshGenerator(int32_t instance);
void Internal_GetMeshVerticeData(int32_t instanceId, G3D::Vector3* pV, G3D::Vector3* pN, int32_t size, int32_t mesh);
void Internal_GetTerraniHeightMap(int32_t instanceId, float* heightMap, int32_t size1, int32_t size2);
//void Internal_GetMeshNormalData(int32_t instance, G3D::Vector3* normals, int32_t size, int32_t mesh);
void Internal_GetMeshUVData(int32_t instance, G3D::Vector2* uvs, int32_t size, int32_t mesh,int32_t uv);
void Internal_GetMeshTrianglesData(int32_t instance, int32_t* triangles, int32_t size, int32_t mesh,int32_t lod);
void Internal_ReloadMeshNormalData(int32_t instanceId, G3D::Vector3 * p, int32_t size, int32_t mesh, int32_t meshEdgePosition);
void Internal_SetMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection);
void Internal_OnNeighborLodChanged(int32_t instanceId, int32_t neighborId);
void Internal_ReleaseAllMeshed();
NS_GNRT_END

#endif
