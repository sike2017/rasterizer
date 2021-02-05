#pragma once
#include <cmath>
#include <assert.h>
#include "color.h"
#include "display/image_png.h"

inline float rangeFloat(const float& min, const float& max, const float& v) {
	if (v < min) return min;
	if (v > max) return max;
	return v;
}

class Texture {
public:
	virtual Color value(float u, float v) const = 0;
};

class CheckerTexture : public Texture {
public:
	CheckerTexture(int _width, int _height, const Color& color0, const Color& color1) {
		width = _width;
		height = _height;
		texture = std::shared_ptr<Color>(new Color[_width * static_cast<long>(_height)](), std::default_delete<Color[]>());

		Color gridColor[2] = { color0, color1 };
		int index = 0;
		for (int x = 0; x < _width; x++) {
			if (x % 10 == 0) {
				index = (index == 0 ? 1 : 0);
			}
			for (int y = 0; y < _height; y++) {
				if (y % 10 == 0) {
					index = (index == 0 ? 1 : 0);
				}
				texture.get()[y * _width + x] = gridColor[index];
			}
		}
	}
	~CheckerTexture() {}

	Color read_texture(int x, int y) const {
		return texture.get()[y * width + x];
	}
	virtual Color value(float u, float v) const override {
		u = rangeFloat(0, 1, u);
		v = rangeFloat(0, 1, v);
		return read_texture(u * (width - 1), v * (height - 1));
	}

	std::shared_ptr<Color> texture;
	int width, height;
};

class ImageTexture : public Texture {
public:
	ImageTexture(const std::string& image_file) {
		image = image_png::load_image(image_file.c_str());
	}
	~ImageTexture() {
		image_png::free_image(&image);
	}
	virtual Color value(float u, float v) const override {
		int i = (u)*image.width;
		int j = (1 - v) * image.height - 0.001;
		if (i < 0) i = 0;
		if (j < 0) j = 0;
		if (j > image.width - 1) j = image.width - 1;
		if (j > image.height - 1) j = image.height - 1;
		float r = int(image.p_buffer[3 * i + 3 * image.width * j]) / 255.0;
		float g = int(image.p_buffer[3 * i + 3 * image.height * j + 1]) / 255.0;
		float b = int(image.p_buffer[3 * i + 3 * image.height * j + 2]) / 255.0;
		return Color(r, g, b);
	}

	image_png::image_t image;
};

class ConstantTexture : public Texture {
public:
	ConstantTexture(const Color& _color) { color = _color; }
	virtual Color value(float u, float v) const override {
		return color;
	}
	Color color;
};

inline void barycentric(const Point2f& q, const Point2f& p0, const Point2f& p1, const Point2f& p2, Point3f* out) {
	float gama = ((p0.y() - p1.y()) * q.x() + (p1.x() - p0.x()) * q.y() + p0.x() * p1.y() - p1.x() * p0.y()) /
		((p0.y() - p1.y()) * p2.x() + (p1.x() - p0.x()) * p2.y() + p0.x() * p1.y() - p1.x() * p0.y());
	float beta = ((p0.y() - p2.y()) * q.x() + (p2.x() - p0.x()) * q.y() + p0.x() * p2.y() - p2.x() * p0.y()) /
		((p0.y() - p2.y()) * p1.x() + (p2.x() - p0.x()) * p1.y() + p0.x() * p2.y() - p2.x() * p0.y());
	float alpha = 1.0f - beta - gama;

	*out = Vector3f(alpha, beta, gama);
}

class Material {
public:
	Material(Texture* _texture) { texture = _texture; }
	virtual Color light_value(const Vertex& hit_point, const Point3f& _light, const Point3f& _eye, const Vector3f& _normal) const = 0;

	Texture* texture;
};

class Blinn_Phong : public Material {
public:
	Blinn_Phong(float _ka, float _kd, float _ks, Texture* ptr_texture) : Material(ptr_texture), ka(_ka), kd(_kd), ks(_ks) {}
	~Blinn_Phong() {}

	virtual Color light_value(const Vertex& hit_point, const Point3f& _light, const Point3f& _eye, const Vector3f& _normal) const override {
		Color color;
		const Color la(1, 1, 1);

		const Color lp(1, 1, 1); // ∑¥…‰π‚—’…´
		const float r = (_light - hit_point.p).length();
		const Vector3f light_direct = _light - hit_point.p;
		const Vector3f view_direct = _eye - hit_point.p;
		const float r2 = r * r;
		const Vector3f h = unit_vector(view_direct + light_direct);
		color =
			la * ka + lp * kd * (lp / r2) * std::max(0.0f, dot(_normal, light_direct)) + ks * (lp / r2) * pow(std::max(0.0f, dot(_normal, h)), 16);
		return color;
	}

	float ka, kd, ks;
	Texture* texture;
};
