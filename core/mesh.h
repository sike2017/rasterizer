#pragma once
#include <vector>
#include "math/monolith_math.h"

typedef Vector3f Point3f;
typedef Vector2f Point2f;

class Vertex {
public:
	Vertex(const Point3f& _p) { p = _p; }
	Point3f p;
};

inline Log& operator<<(Log& log, const Vertex& v) {
	log << v.p;
	return log;
}

template<typename T>
class base_list : public std::vector<T> {
public:
	void add(T a) {
		push_back(a);
	}
};

template<typename T>
class Triple {
public:
	Triple<T>() {}
	Triple<T>(T v0, T v1, T v2) { e[0] = v0; e[1] = v1; e[2] = v2; }
	
	T& operator[](int index) { return e[index]; }
	const T operator[](int index) const { return e[index]; }
	T e[3];
};

class Triangle {
public:
	Triangle(const Triple<Vertex*>& _v, const Triple<Point2f*>& _vt, const Triple<Vector3f*>& _vn) { v = _v; vt = _vt; vn = _vn; }
	~Triangle() {
		delete v[0];
		delete v[1];
		delete v[2];
		delete vt[0];
		delete vt[1];
		delete vt[2];
		delete vn[0];
		delete vn[1];
		delete vn[2];
		v[0] = v[1] = v[2] = nullptr;
		vt[0] = vt[1] = vt[2] = nullptr;
		vn[0] = vn[1] = vn[2] = nullptr;
	}

	// 把 Triangle 的顶点位置与矩阵相乘
	void mul(const Matrix4x4f& m) {
		v[0]->p = m * (v[0]->p);
		v[1]->p = m * (v[1]->p);
		v[2]->p = m * (v[2]->p);
	}

	Triple<Vertex*> v;
	Triple<Point2f*> vt;
	Triple<Vector3f*> vn;
};

class Mesh : public base_list<Triangle*> {
public:
	Mesh() {}
	~Mesh() {
		for (Triangle* p : *this) {
			delete p;
		}
	}
};

typedef base_list<Mesh*> MeshList;
