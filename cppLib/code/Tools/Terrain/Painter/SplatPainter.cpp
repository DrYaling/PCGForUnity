#include "SplatPainter.h"

namespace generator
{

	SplatPainter::SplatPainter()
	{
	}


	SplatPainter::~SplatPainter()
	{
	}
	//ensure alphaMap has enough space
	inline void SplatPainter::Normalize(int32_t x, int32_t y, int32_t splatCount, int32_t splatIndex, float * alphaMap)
	{
		if (splatIndex < 0 || splatIndex >= splatCount)
		{
			return;
		}
		if (!alphaMap)
			return;
		int	 line = GetSplatMapIndex(x, y, 0, m_nSize, m_nAlphaCount); ;
		float newAlpha = alphaMap[line + splatIndex];
		float totalAlphaOthers = 0;
		for (int32_t i = 0; i < splatCount; i++)
		{
			if (i != splatIndex)
				totalAlphaOthers += alphaMap[line + i];
		}

		if (totalAlphaOthers > 0.01)
		{
			float adjust = (1.0F - newAlpha) / totalAlphaOthers;
			for (int a = 0; a < splatCount; a++)
			{
				if (a != splatIndex)
					alphaMap[line + a] *= adjust;
			}
		}
		else
		{
			for (int a = 0; a < splatCount; a++)
			{
				alphaMap[line + a] = a == splatIndex ? 1.0F : 0.0F;
			}
		}

	}
	void SplatPainter::Paint(int xCenter, int yCenter, int splatIndex)
	{
		if (splatIndex >= m_nAlphaCount)
		{
			return;
		}

		int intRadius = m_nSize / 2;
		int intFraction = m_nSize % 2;
		int xmin = generator::clamp<int>(xCenter - intRadius, 0, m_nSize - 1);
		int ymin = generator::clamp<int>(yCenter - intRadius, 0, m_nSize - 1);
		int xmax = generator::clamp<int>(xCenter + intRadius + intFraction, 0, m_nSize);
		int ymax = generator::clamp<int>(yCenter + intRadius + intFraction, 0, m_nSize);
		int width = xmax - xmin;
		int height = ymax - ymin;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int xBrushOffset = (xmin + x) - (xCenter - intRadius + intFraction);
				int yBrushOffset = (ymin + y) - (yCenter - intRadius + intFraction);
				float brushStrength = m_pBrush->GetStrength(xBrushOffset, yBrushOffset);

				// Paint with brush
				float newAlpha = m_pAlphaMap[GetSplatMapIndex(y, x, splatIndex, m_nSize, m_nAlphaCount)] + brushStrength;
				m_pAlphaMap[GetSplatMapIndex(y,x,splatIndex,m_nSize,m_nAlphaCount)]= newAlpha;
				Normalize(x, y, splatIndex, splatIndex, m_pAlphaMap);
			}
		}
	}
}