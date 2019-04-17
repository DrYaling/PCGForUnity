#ifndef AUTOMATIC_PAINTER_H
#define AUTOMATIC_PAINTER_H
#include "generator.h"
#include "define.h"
#include "Painter.h"
namespace generator
{
	class AutomaticPainter
	{
	public:
		AutomaticPainter();
		~AutomaticPainter();
		void Switch(PainterType type);
	private:
		Painter * m_pPainter;
	};
}
#endif

