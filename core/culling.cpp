#include "culling.h"

bool cull(const Vertex& v) { // ndc culling
	if (v.p.x() < 0 && v.p.x() > 1 && v.p.y() < 0 && v.p.y() > 1 && v.p.z() < 0 && v.p.z() > 1) {
		return true;
	}
	return false;
}

int clip(const Primitive& pr, const Vertex& v, Primitive* p0, Primitive* p1) { // ndc clipping
	for (int j = 0; j < 3; j++) {
		if (v.p[j] < -1.0f || v.p[j] > 1.0f) {
			return 0;
		}
	}
	*p0 = pr;
	return 1;
}
