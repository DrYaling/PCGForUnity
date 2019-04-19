#include "AutomaticPainter.h"
#include "SplatPainter.h"
#include "Logger/Logger.h"
namespace generator
{
	AutomaticPainter::AutomaticPainter() :
		m_pPainter(nullptr)
	{
		Switch(generator::PainterType::SplatPainter);
	}


	AutomaticPainter::~AutomaticPainter()
	{
	}
	void AutomaticPainter::Init(float * heightMap, int32_t heightSizeXY, float * alphaMap, int32_t alphaSizeXY, int32_t splatCount)
	{
		if (!heightMap || !alphaMap)
		{
			LogError("height map or alpha0 map error!");
			return;
		}
		if (m_pPainter)
		{
			m_pPainter->Init(alphaMap, alphaSizeXY, splatCount);
			m_pPainter->ResetBrush(5);
		}
		m_pHeightMap = heightMap;
		m_nSize = heightSizeXY;
		m_nMax = m_nSize - 1;
	}
	void AutomaticPainter::Switch(PainterType type)
	{
		switch (type)
		{
		case generator::PainterType::SplatPainter:
			if (!m_pPainter || dynamic_cast<SplatPainter*>(m_pPainter) == nullptr)
			{
				safe_delete(m_pPainter);
				m_pPainter = new SplatPainter();
			}
			break;
		case generator::PainterType::DetailPainter:
			break;
		default:
			break;
		}
	}
	void AutomaticPainter::DrawDetail()
	{
	}
	void AutomaticPainter::DrawSplatMap()
	{
		if (!m_pPainter)
		{
			LogErrorFormat("Painter Not Initilized yet!");
			return;
		}
		SplatPainter* painter = dynamic_cast<SplatPainter*>(m_pPainter);
		if (!painter)
		{
			LogErrorFormat("DrawSplatMap need SplatPainter!");
			return;
		}
		InitSplatMap();
		int brushSize = painter->GetBrushSize() / 2;
		int intFraction = painter->GetBrushSize() % 2;
		if (brushSize < 1)
		{
			brushSize = 1;
		}
		int alphaSize = painter->GetAlphaSize();
		float scale = m_nSize / (float)alphaSize;
		//int alphaCount = painter->GetAlphaCount();
		//LogFormat("alphaSize  %d,alphaCount %d,brush size %d,scale %f", alphaSize, alphaCount, brushSize, scale);
		int32_t x = 0, y = 0;
		for (y = 0; y < alphaSize; y += brushSize)
		{
			for (x = 0; x < alphaSize; x += brushSize)
			{
				Paint(x, y, brushSize, scale);
			}
		}
		y = alphaSize - intFraction - 1;
		x = alphaSize - intFraction - 1;
		for (; y < alphaSize; y += brushSize)
		{
			for (; x < alphaSize; x += brushSize)
			{
				Paint(x, y, brushSize, scale);
			}
		}
		//LogFormat("smooth cnt %d,rough cnt %d,sharp cnt %d,lh cnt %d",smoothCnt,roughCnt,sharpCnt,hlCnt);
	}
	void AutomaticPainter::InitSplatMap()
	{
		SplatPainter* painter = dynamic_cast<SplatPainter*>(m_pPainter);
		if (!painter)
		{
			LogErrorFormat("DrawSplatMap need SplatPainter!");
			return;
		}
		float* map = painter->GetAlphaMap();
		int32_t size = painter->GetAlphaSize();
		int32_t count = painter->GetAlphaCount();
		for (int32_t x = 0; x < size; x++)
		{
			for (int32_t y = 0; y < size; y++)
			{
				for (int32_t a = 0; a < count; a++)
				{
					map[GetSplatMapIndex(x, y, a, size, count)] = a == 0 ? 1.0f : 0;
				}
			}
		}
	}
	void AutomaticPainter::Paint(int32_t x, int32_t y, int32_t brushSize, float scale)
	{
		return;
		SplatPainter* painter = dynamic_cast<SplatPainter*>(m_pPainter);
		int heightMapX = 0, heightMapY = 0;
		heightMapX = (int)(scale*x);
		heightMapY = (int)(scale*y);
		generator_clamp(heightMapX, 0, m_nMax);
		generator_clamp(heightMapY, 0, m_nMax);
		float height = m_pHeightMap[GetHeightMapIndex(heightMapX, heightMapY)] * MAX_MAP_HEIGHT;

		if (height > TERRAIN_HIGH_NO_GRASS_HEIGHT)
		{
			painter->SetBaseBrushStrength(1);
			painter->Paint(x, y, 1);
		}
		else if (height < TERRAIN_LOW_NO_GRASS_HEIGHT)
		{
			painter->SetBaseBrushStrength(1);
			painter->Paint(x, y, 1);
		}
		else
		{
			//if smooth nearby,paint grass
			//if rough nearby,paint random grass and rock
			//if sharp nearby,paint rock with high level
			auto type = GetTerrainStateNear(heightMapX, heightMapY, height);
			float alpha0 = painter->GetAlpha(x - brushSize, y - brushSize, 0);
			bool alpha0_change = false;
			switch (type)
			{
			case TerrainSmoothType::Smooth:
				if (alpha0 > 0.15f)
				{
					alpha0_change = GetChance(0.85f);
				}
				else
				{
					alpha0 = alpha0 > 0.1f ? 0.65f : 0.55f;
					alpha0_change = GetChance(alpha0);
				}
				break;
			case TerrainSmoothType::Rough:
			{
				if (alpha0 > 0.25f)
				{
					alpha0_change = GetChance(0.45f);
				}
				else
				{
					alpha0 = alpha0 > 0.125 ? 0.65 : 0.45;
					alpha0_change = GetChance(alpha0);
				}
			}
			break;
			case TerrainSmoothType::Sharp:
			{
				if (alpha0 > 0.85f)
				{
					alpha0_change = GetChance(0.45f);
				}
				else
				{
					alpha0 = alpha0 > 0.15 ? 0.45 : 0.35;
					alpha0_change = GetChance(alpha0);
				}
			}
			break;
			default:
				break;
			}

			if (alpha0_change)
			{
				painter->SetBaseBrushStrength(1);
				painter->Paint(x, y, 0);
			}
			else
			{
				//painter->SetBaseBrushStrength(1.0f);
				//painter->Paint(x, y, 0);
			}
			/*if (type == TerrainSmoothType::Smooth && x % 2 == 0)
			{
				LogFormat("smooth x %d,y %d is %f,change %d", x, y, painter->GetAlpha(x, y, 0), alpha0_change);;
			}*/
		}
	}
	TerrainSmoothType AutomaticPainter::GetTerrainStateNear(int32_t x, int32_t y, float height)
	{
		float variance = 0;
		for (int32_t offsetY = -2; offsetY <= 2; offsetY++)
		{
			int ry = y + offsetY;
			generator_clamp(ry, 0, m_nMax);
			for (int32_t offsetX = -2; offsetX <= 2; offsetX++)
			{
				int rx = x + offsetX;
				generator_clamp(rx, 0, m_nMax);
				if (rx == x && ry == y)
				{
					continue;;
				}
				float offsetHeight = height - m_pHeightMap[GetHeightMapIndex(rx, ry)] * MAX_MAP_HEIGHT;
				variance += offsetHeight * offsetHeight;
			}
		}
		variance /= 24.0f;
		//LogFormat("variance %f", variance);
		if (variance <= (int32_t)TerrainSmoothType::Smooth)
		{
			return TerrainSmoothType::Smooth;
		}
		else if (variance <= (int32_t)TerrainSmoothType::Rough)
		{
			return TerrainSmoothType::Rough;
		}
		else
		{
			return TerrainSmoothType::Sharp;
		}
	}
}
