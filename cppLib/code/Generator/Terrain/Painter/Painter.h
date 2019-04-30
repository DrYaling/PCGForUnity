#ifndef PAINTER_H
#define PAINTER_H
#include "define.h"
namespace generator
{
	//���λ���������
	enum class PainterType
	{
		SplatPainter = 0,
		DetailPainter
	};
	class Painter
	{
	public:
		Painter();
		virtual ~Painter();
		virtual void Init(float* map, int32_t sizeXY, int32_t arg) {}
		virtual void ResetBrush(int32_t brushSize) {}
	};
}
#endif

