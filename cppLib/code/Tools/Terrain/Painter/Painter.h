#ifndef PAINTER_H
#define PAINTER_H
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
	};
}
#endif

