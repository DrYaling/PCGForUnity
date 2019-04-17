#include "SplatPainter.h"

namespace generator
{

	SplatPainter::SplatPainter():
		m_pAlphaMap(nullptr),
		m_nAlphaCount(0),
		m_nSize(0)
	{
		m_pBrush = new PainterBrush();
	}


	SplatPainter::~SplatPainter()
	{
		safe_delete(m_pBrush);
	}
	void SplatPainter::Init(float * alphaMap, int32_t sizeXY, int32_t splatCount)
	{
		m_nAlphaCount = splatCount;
		m_nSize = sizeXY;
		m_pAlphaMap = alphaMap;
		m_pBrush->Initilize(BrushStyle::Circle_middle, 5);
	}
	void SplatPainter::ResetBrush(int32_t brushSize)
	{
		m_pBrush->Initilize(BrushStyle::Circle_middle, brushSize);
	}
	//ensure alphaMap has enough space
	inline void SplatPainter::Normalize(int32_t x, int32_t y, int32_t splatIndex)
	{
		if (splatIndex < 0 || splatIndex >= m_nAlphaCount)
		{
			return;
		}
		int	 line = GetSplatMapIndex(x, y, 0, m_nSize, m_nAlphaCount); ;
		float newAlpha = m_pAlphaMap[line + splatIndex];
		float totalAlphaOthers = 0;
		for (int32_t i = 0; i < m_nAlphaCount; i++)
		{
			if (i != splatIndex)
				totalAlphaOthers += m_pAlphaMap[line + i];
		}

		if (totalAlphaOthers > 0.01f)
		{
			float adjust = (1.0F - newAlpha) / totalAlphaOthers;
			for (int a = 0; a < m_nAlphaCount; a++)
			{
				if (a != splatIndex)
					m_pAlphaMap[line + a] *= adjust;
			}
		}
		else
		{
			for (int a = 0; a < m_nAlphaCount; a++)
			{
				m_pAlphaMap[line + a] = a == splatIndex ? 1.0F : 0.0F;
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
				int index = GetSplatMapIndex(y, x, splatIndex, m_nSize, m_nAlphaCount);
				// Paint with brush
				m_pAlphaMap[index] += brushStrength;
				Normalize(x, y, splatIndex);
			}
		}
	}
}