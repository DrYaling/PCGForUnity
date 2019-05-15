#ifndef AUTOMATIC_PAINTER_H
#define AUTOMATIC_PAINTER_H
#include "generator.h"
#include "define.h"
#include "Painter.h"
namespace generator
{
	class AutomaticPainter
	{
	public:
		AutomaticPainter();
		~AutomaticPainter();
		void Init(float* heightMap, int32_t heightSizeXY, float* alphaMap, int32_t alphaSizeXY, int32_t splatCount = 4);
		void Switch(PainterType type);
		void DrawDetail();
		void DrawSplatMap();
	private:
		void InitSplatMap() const;
		void Paint(int32_t x, int32_t y, int32_t brushSize,float scale) const;
		TerrainSmoothType GetTerrainStateNear(int32_t x, int32_t y,float height);
	private:
		Painter * m_pPainter;
		float* m_pHeightMap;
		int32_t m_nSize;//height map size
		int32_t m_nMax;
	};
}
#endif

