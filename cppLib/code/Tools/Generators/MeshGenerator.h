#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H
#include "G3D/Vector3.h"
#include "generator.h"
using namespace G3D;
NS_GNRT_START

void InitGenerator(int32_t seed, const int32_t* args, int32_t argSize, bool optimalize);
void GenMeshData(int32_t type,int32_t* vSize);
void GetGeneratorData(Vector3* pV, int32_t* pI, int arg0);
void GetGeneratorTrianglesData(int32_t* pI, int arg0);
int32_t GetGeneratorVerticesData(Vector3* pV,Vector3* pN, int index);
void ReleaseGenerator();
NS_GNRT_END

#endif
