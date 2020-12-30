#include "rasterizer.h"

void renderThreadCallBack(const DrawCommand& d, void* p, Primitive* primitive, Primitive* p0, Primitive* p1) {
	reinterpret_cast<Dwindow*>(p)->processTriangle(d, primitive, p0, p1);
}