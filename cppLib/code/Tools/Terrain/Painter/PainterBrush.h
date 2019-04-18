#ifndef PAINTER_BRUSH_H
#define PAINTER_BRUSH_H
#include "define.h"
#include <vector>
namespace generator
{
	enum class BrushStyle
	{
		Circle_ful,//实心园
		Circle_middle,//圆形实心，周围模糊
		Square_full,//实心正方形
		Square_middle,//正方形中心实心，周围模糊
	};
	/*
	地形刷
	详见Unity Terrain Editor Inspector的 Brushes（PainDail 或者Tree 等先的刷子图片）
	*/
	class PainterBrush
	{
	public:
		PainterBrush();
		~PainterBrush();
		float GetStrength(int32_t xOffset, int32_t yOffset);
		void Initilize(BrushStyle style, int32_t size);
		int32_t GetBrushSize() { return m_nSize; }
		static int32_t GetMinBrushSize() { return nBrushMinSize; }
	private:
		int32_t m_nSize;
		int32_t m_nMax;
		std::vector<float> m_vStrength;
		static const int nBrushMinSize = 3;
	};


}
#endif