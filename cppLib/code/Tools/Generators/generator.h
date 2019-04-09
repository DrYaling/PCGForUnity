#ifndef GENERATOR_H
#define GENERATOR_H
#include <thread>
#include <stdlib.h>
#include <vector>
#include <map>
#include <mutex>
#include <stdio.h>
#include "G3D/Vector3.h"
#include "define.h"
#define NS_GNRT_START namespace generator{\

#define NS_GNRT_END }\

#define  UNITY_CORE 1

static inline int _irandom(int min, int max)
{
	if (min >= max)
		return min;
	return std::rand() % (max - min) + min;
}
static inline float _frandom(int min, int max)
{
	if (min >= max)
		return (float)min;
	return (float)(std::rand() % (max - min) + min) + (std::rand() % 10000) / 10000.0f;
}
static inline float _frandom_f(float min, float max)
{
	if (min >= max)
	{
		return min;
	}
	int i(min * 10000), a(max * 10000);
	return (float)(_irandom(i, a) / 10000.0f);
}
static inline void setRandomSeed(int seed)
{
	std::srand(seed);
}
//地形
enum eGeographyType :int16_t
{
	e_geo_trench = 0x0000,
	e_geo_deepSea = 0x0001,
	e_geo_shallowSea = 0x0002,
	e_geo_sea_land = 0x0004,
	e_geo_flatlands = 0x0008,
	e_geo_basin = 0x0010,
	e_geo_foothill = 0x0020,
	e_geo_montains = 0x0040,
	e_geo_high_montains = 0x0080,
	e_geo_heightLand = 0x0100,
};
//sub geography property
enum eGeographySubType :int16_t
{
	eg_sub_volcano = 1,//火山
	eg_sub_marsh = 2,//沼泽
	eg_sub_forest = 3,//森林
	eg_sub_desert = 4,//沙漠
	eg_sub_grassland = 5,//草原
	eg_sub_lake = 6,//湖泊
};
//地理属性
typedef struct geographyProperties
{
	uint64_t gType : 2;//eGeographyType
	uint64_t gSubType : 2;//eGeographySubType
	uint64_t e : 8;

}*pGeographyProperties;
#if 0
[StructLayout(LayoutKind.Sequential)]
class TerrianData
{
	private bool _useUV;
	private int _meshCount;
	private int _lodCount;
	Vector3[][] _vertices;
	Vector3[][] _normals;
	Vector2[][][] _uvs;
	int[][][] _triangles;
}
#endif
class TerrianDataBinding
{
public:
	bool isReadable;
	bool useUv;
	int32_t meshCount;
	int32_t lodCount;
	G3D::Vector3** vertices;
	G3D::Vector3** normals;
	G3D::Vector2*** uvs;
	int32_t*** triangles;
	TerrianDataBinding(int32_t maxMesh, int32_t maxLodCount, bool Uv)
	{
		vertices = new G3D::Vector3*[maxMesh];
		normals = new G3D::Vector3*[maxMesh];
		meshCount = maxMesh;
		lodCount = maxLodCount;
		useUv = Uv;
		if (Uv)
		{
			uvs = new G3D::Vector2**[maxMesh];
			for (int i = 0; i < maxMesh; i++)
			{
				uvs[i] = new G3D::Vector2*[4];
			}
		}
		triangles = new int32_t**[maxMesh];
		for (int i = 0; i < maxMesh; i++)
		{
			triangles[i] = new int32_t*[maxLodCount];
		}
	}

	void SetTerrianVerticesData(G3D::Vector3* p, int32_t size, int32_t mesh)
	{
		if (mesh >= 0 && mesh < meshCount)
		{
			vertices[mesh] = p;
		}
	}
	void SetMeshNormalData(G3D::Vector3* p, int32_t size, int32_t mesh)
	{
		if (mesh >= 0 && mesh < meshCount)
		{
			normals[mesh] = p;
		}
	}
	void SetMeshUVData(G3D::Vector2* p, int32_t size, int32_t mesh, int32_t uv)
	{
		if (mesh >= 0 && mesh < meshCount && uv >= 0 && uv < 4)
		{
			uvs[mesh][uv] = p;
		}
	}
	void SetMeshTriangleData(int32_t* p, int32_t size, int32_t mesh, int32_t lod)
	{
		if (mesh >= 0 && mesh < meshCount && lod >= 0 && lod < lodCount)
		{
			triangles[mesh][lod] = p;
		}
	}
	~TerrianDataBinding()
	{
		//remove managed memeroy
		for (int i = 0; i < meshCount; i++)
		{
			vertices[i] = nullptr;
			normals[i] = nullptr;
			if (useUv)
			{
				for (int j = 0; j < 4; j++)
				{
					uvs[i][j] = nullptr;
				}
			}
			for (int j = 0; j < lodCount; j++)
			{
				triangles[i][j] = nullptr;
			}
		}
		safe_delete_array(vertices);
		safe_delete_array(normals);
		safe_delete_array(uvs);
		safe_delete_array(triangles);
	}
};
class TerrianGenerator
{
public:
	TerrianGenerator() {};
	virtual ~TerrianGenerator() {};

private:

};
#define MAX_MESH_COUNT 36
#define  MAX_MESH_VERTICES 65000
typedef void(__stdcall * ResizeIndicesCallBack)(int32_t type, int32_t mesh, int32_t lod, int32_t size);
#define  meshTopologyVertice 0
#define  meshTopologyTriangle 1
#define meshTopologyUV 2
// typedef geography* pGeography;
#endif