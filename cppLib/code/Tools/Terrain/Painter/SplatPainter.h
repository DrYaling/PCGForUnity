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
		int32_t GetBrushSize()
		{
			if (m_pBrush)
			{
				return m_pBrush->GetBrushSize();
			}
			return PainterBrush::GetMinBrushSize();
		}
		void SetBaseBrushStrength(float strength) { m_fBrushStrength = strength; }
		float GetBaseBrushStrength() { return m_fBrushStrength; }
		float GetAlpha(int32_t x, int32_t y, int32_t splatIndex)
		{
			if (splatIndex >= m_nAlphaCount)
			{
				return 0;
			}
			generator_clamp(x, 0, m_nSize - 1);
			generator_clamp(y, 0, m_nSize - 1);
			return m_pAlphaMap[GetSplatMapIndex(x, y, splatIndex, m_nSize, m_nAlphaCount)];
		}
	private:
		PainterBrush * m_pBrush;
		float* m_pAlphaMap;
		int32_t m_nAlphaCount;
		int32_t m_nSize;
		float m_fBrushStrength;
	};
}
#endif