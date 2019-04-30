#include "PainterBrush.h"
#include "generator.h"
#include "Logger/Logger.h"
namespace generator
{
	PainterBrush::PainterBrush() :m_vStrength(nBrushMinSize*nBrushMinSize)
	{
	}

	PainterBrush::~PainterBrush()
	{
	}
	float PainterBrush::GetStrength(int32_t xOffset, int32_t yOffset)
	{
		return 1.0f;
		generator_clamp(xOffset, 0, m_nMax);
		generator_clamp(yOffset, 0, m_nMax);
		return m_vStrength[yOffset*m_nSize + xOffset];
	}
	void PainterBrush::Initilize(BrushStyle style, int32_t size)
	{
		m_vStrength.resize(size*size);
		m_nSize = size;
		m_nMax = size - 1;
		if (size <= nBrushMinSize)
		{
			memset(m_vStrength.data(), 1, sizeof(float)*m_vStrength.size());
			return;
		}
		memset(m_vStrength.data(), 1, sizeof(float)*m_vStrength.size());
		return;
		switch (style)
		{
		case generator::BrushStyle::Circle_ful:
		case generator::BrushStyle::Square_full:
			memset(m_vStrength.data(), 1, sizeof(float)*m_vStrength.size());
			break;
		case generator::BrushStyle::Circle_middle:
		case generator::BrushStyle::Square_middle:
		{
			float minStrength = 0.0f;
			float middle = size / 2;
			float radius = size * 0.5f;
			for (int32_t y = 0; y < size; y++)
			{
				for (int32_t x = 0; x < size; x++)
				{
					float distance = GetDistance2D(x, y, middle, middle);
					float value = (radius - distance) / radius;
					value = value <= minStrength ? minStrength : value;
					m_vStrength[y*size + x] = value;
					//LogFormat("Brush Strength %f",value);
				}
			}
		}
		break;
		default:
			break;
		}
	}
}
