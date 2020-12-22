#include "transformation.h"
#include "mesh.h"

class Camera : public Transformer {
public:
	Camera(const Point3f& _lookfrom, const Point3f& lookat, const Vector3f& vup) {
		Vector3f direction = lookat - _lookfrom;
		w = -direction / direction.length();
		Vector3f fraction = cross(vup, w);
		u = fraction / fraction.length();
		v = cross(w, u);
		lookfrom = _lookfrom;
	}
	~Camera() {}

	virtual Matrix4x4f get_transform() override {
		return Matrix4x4f(u.x(), u.y(), u.z(), 0,
			v.x(), v.y(), v.z(), 0,
			w.x(), w.y(), w.z(), 0,
			0, 0, 0, 1) * Matrix4x4f (1, 0, 0, -lookfrom.x(),
				0, 1, 0, -lookfrom.y(),
				0, 0, 1, -lookfrom.z(),
				0, 0, 0, 1);
	}

	Point3f lookfrom;
	Vector3f u, v, w;
};

class Perspective : public Transformer {
public:
	Perspective(float fovy, float aspect, float _zNear, float _zFar): zNear(_zNear), zFar(_zFar) { // aspect = width / height
		top = - tan(fovy * 0.5 * M_PI / 180) * _zNear;
		bottom = -top;
		right = top * aspect;
		left = -right;
	}
	~Perspective() {}

	virtual Matrix4x4f get_transform() override {
		return Matrix4x4f(2 * zNear / (right - left), 0, (left + right) / (left - right), 0,
			0, 2 * zNear / (top - bottom), (bottom + top) / (bottom - top), 0,
			0, 0, (zFar + zNear) / (zNear - zFar), 2 * zFar * zNear / (zFar - zNear),
			0, 0, 1, 0);
	}

	float zNear, zFar, bottom, top, left, right;
};

class Viewport : public Transformer {
public:
	Viewport(int _width, int _height): width(_width), height(_height) {}
	virtual Matrix4x4f get_transform() override {
		float half_width = width / 2.0;
		float half_height = height / 2.0;
		return Matrix4x4f(half_width, 0, 0, half_width,
			0, half_height, 0, half_height,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	int width, height;
};
