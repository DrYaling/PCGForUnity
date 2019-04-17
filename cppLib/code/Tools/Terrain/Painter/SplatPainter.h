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
	class SplatPainter :public Painter
	{
	public:
		SplatPainter();
		~SplatPainter();
		void Init(float* alphaMap, int32_t sizeXY, int32_t splatCount);
		void ResetBrush(int32_t brushSize);
		inline void Normalize(int32_t x, int32_t y, int32_t splatIndex);
		void Paint(int xCenter, int yCenter, int splatIndex);
		float* GetAlphaMap() { return m_pAlphaMap; }
		int32_t GetAlphaCount() { return m_nAlphaCount; }
		int32_t GetAlphaSize() { return m_nSize; }
		int32_t GetAlphaMapSize() { return m_nSize * m_nSize*m_nAlphaCount; }
	private:
		PainterBrush * m_pBrush;
		float* m_pAlphaMap;
		int32_t m_nAlphaCount;
		int32_t m_nSize;
	};
}
#endif