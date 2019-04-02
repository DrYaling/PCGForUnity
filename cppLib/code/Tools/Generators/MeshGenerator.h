#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H
#include "TerrianGenerator/Mountain.h"
#include "G3D/Vector3.h"
#include "generator.h"
using namespace G3D;
NS_GNRT_START

void InitGenerator(Vector3 pos, int32_t seed, int32_t depth, int32_t step, int32_t maxHeight, bool usePerlin);
void GenrateMountain(int32_t& vSize,int32_t& idxSize);
void GetGeneratorData(Vector3* pV,int32_t* pI);
void ReleaseGenerator();
NS_GNRT_END

#endif
