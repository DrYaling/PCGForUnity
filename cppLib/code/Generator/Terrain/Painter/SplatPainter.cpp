#include "SplatPainter.h"
#include "Logger/Logger.h"
namespace generator
{

	SplatPainter::SplatPainter() :
		m_pAlphaMap(nullptr),
		m_nAlphaCount(0),
		m_fBrushStrength(1.0f),
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
	inline void SplatPainter::Normalize(int32_t x, int32_t y, float newAlpha, int32_t splatIndex)
	{
		int	 line = GetSplatMapIndex(y, x, 0, m_nSize, m_nAlphaCount); ;
		generator_clamp(newAlpha,0,1.0f);
		m_pAlphaMap[line + splatIndex] = newAlpha;
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
				{
					m_pAlphaMap[line + a] *= adjust;
				}
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
		int brushSize = m_pBrush->GetBrushSize();
		int intRadius = brushSize / 2;
		int intFraction = brushSize % 2;
		int xmin = generator::clamp<int>(xCenter - intRadius, 0, m_nSize - 1);
		int ymin = generator::clamp<int>(yCenter - intRadius, 0, m_nSize - 1);
		int xmax = generator::clamp<int>(xCenter + intRadius + intFraction, 0, m_nSize);
		int ymax = generator::clamp<int>(yCenter + intRadius + intFraction, 0, m_nSize);
		int width = xmax - xmin;
		int height = ymax - ymin;
		int	 nMax = m_nSize - 1;
		int rx, ry;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int xBrushOffset = (xmin + x) - (xCenter - intRadius + intFraction);
				int yBrushOffset = (ymin + y) - (yCenter - intRadius + intFraction);
				rx = xBrushOffset + xCenter;
				ry = yBrushOffset + yCenter;
				generator_clamp(rx, 0, nMax);
				generator_clamp(ry, 0, nMax);
				float brushStrength = m_pBrush->GetStrength(xBrushOffset, yBrushOffset)*m_fBrushStrength;
				int index = GetSplatMapIndex(ry, rx, splatIndex, m_nSize, m_nAlphaCount);
				// Paint with brush
				Normalize(rx, ry, m_pAlphaMap[index] += brushStrength, splatIndex);
				/*if (splatIndex == 0)
				{
				int other = 1;
				LogFormat("alpha %d at x %d,y %d,index %d,str %f,is %f,other %d index %d,alpha %f", splatIndex, rx, ry, index, brushStrength, m_pAlphaMap[index], other, GetSplatMapIndex(ry, rx, other, m_nSize, m_nAlphaCount), m_pAlphaMap[GetSplatMapIndex(ry, rx, other, m_nSize, m_nAlphaCount)]);
				}*/
			}
		}
	}
}