#pragma once
#include "mesh.h"

class Primitive {
public:
	Primitive() {}
	Primitive(const Triangle& tg) {
		v[0] = *tg.v[0];
		v[1] = *tg.v[1];
		v[2] = *tg.v[2];
		vt[0] = *tg.vt[0];
		vt[1] = *tg.vt[1];
		vt[2] = *tg.vt[2];
		vn[0] = *tg.vn[0];
		vn[1] = *tg.vn[1];
		vn[2] = *tg.vn[2];
	}

	Triple<Vertex> v;
	Triple<Point2f> vt;
	Triple<Point3f> vn;
};
