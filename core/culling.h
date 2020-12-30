#pragma once
#include "primitive.h"

bool cull(const Vertex& v);

int clip(const Primitive& pr, const Vertex& v, Primitive* p0, Primitive* p1);
