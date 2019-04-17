#ifndef PAINTER_H
#define PAINTER_H
namespace generator
{
	//地形绘制器类型
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

