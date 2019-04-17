#ifndef SPLAT_PAINTER_H
#define SPLAT_PAINTER_H
#include "generator.h"
#include "define.h"
#include "PainterBrush.h"
#include "Painter.h"
namespace generator
{
	/**************************************************************************/
	/* alphaMap Painter unity的SplatMaterial会自动绘制图片，只需要设置图片即可*/
	/**************************************************************************/
	class SplatPainter:public Painter
	{
		public:
			SplatPainter();
			~SplatPainter();
			inline void Normalize(int32_t x, int32_t y,int32_t splatCount, int32_t splatIndex, float* alphaMap);
			void Paint(int xCenter, int yCenter, int splatIndex);
	private:
		PainterBrush * m_pBrush;
		float* m_pAlphaMap;
		int32_t m_nAlphaCount;
		int32_t m_nSize;
	};
}
#endif