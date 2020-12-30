#pragma once
#include <vector>
#include "rz_types.h"
#include "triangle.h"
#include "material.h"

template<typename T>
class base_list : public std::vector<T> {
public:
	void add(T a) {
		push_back(a);
	}
};

class Mesh : public base_list<Triangle*> {
public:
	Mesh() { ptr_mat = nullptr; }
	~Mesh() {
		for (Triangle* p : *this) {
			delete p;
		}
	}
	void set_material(Material* ptr_texture) {
		ptr_mat = ptr_texture;
	}

	base_list<Vertex*> vArray;
	base_list<Point2f*> vtArray;
	base_list<Vector3f*> vnArray;
	Material* ptr_mat;
};

typedef base_list<Mesh*> MeshList;
