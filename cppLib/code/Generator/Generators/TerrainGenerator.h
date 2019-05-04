#if false
#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H
#include "G3D/Vector3.h"
#include "generator.h"
using namespace G3D;
NS_GNRT_START

void Internal_InitTerrianMesh(int32_t instanceId,int32_t* args,int32_t argsize, float* heightMap, int32_t heightMapSize, MeshInitilizerCallBack cb);
void Internal_StartGenerateOrLoad(int32_t instanceId);
void Internal_RegisterTerrianMeshBinding(int32_t instance);
void Internal_ReleaseGenerator(int32_t instance);
void Internal_FlushMeshGenerator(int32_t instance);
void Internal_GetTerraniHeightMap(int32_t instanceId, float* heightMap, int32_t size1, int32_t size2);
//void Internal_GetMeshNormalData(int32_t instance, G3D::Vector3* normals, int32_t size, int32_t mesh);
void Internal_SetMeshNeighbor(int32_t instanceId, int32_t neighborId, int32_t neighborDirection);
void Internal_InitTerrainPainter(int32_t instanceId, float * alphaMap, int32_t sizeXy, int32_t splatCount);
void Internal_ReleaseAllMeshed();
NS_GNRT_END

#endif
#endif