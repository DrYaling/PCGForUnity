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
			LogError("height map or alpha map error!");
			return;
		}
		if (m_pPainter)
		{
			m_pPainter->Init(alphaMap, alphaSizeXY, splatCount);
			m_pPainter->ResetBrush(5);
		}
		m_pHeightMap = heightMap;
		m_nHeightMapSize = heightSizeXY;
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
}
