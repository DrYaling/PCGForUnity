#ifndef PAINTER_BRUSH_H
#define PAINTER_BRUSH_H
#include "define.h"
#include <vector>
namespace generator
{
	enum class BrushStyle
	{
		Circle_ful,//ʵ��԰
		Circle_middle,//Բ��ʵ�ģ���Χģ��
		Square_full,//ʵ��������
		Square_middle,//����������ʵ�ģ���Χģ��
	};
	/*
	����ˢ
	���Unity Terrain Editor Inspector�� Brushes��PainDail ����Tree ���ȵ�ˢ��ͼƬ��
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