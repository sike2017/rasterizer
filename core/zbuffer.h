#pragma once
#include <memory>
#include <assert.h>
#include <mutex>
#include "material.h"

class ZBuffer
{
public:
	ZBuffer(int _width, int _height) {
		width = _width;
		height = _height;
		long long size = static_cast<long long>(_width) * _height;
		zbuffer = std::shared_ptr<float>(new float[size](), std::default_delete<float[]>());
		resetBuffer();
	}
	~ZBuffer() {}

	inline float& operator()(int x, int y) {
		return zbuffer.get()[y * width + x];
	}

	inline bool update(int x, int y, float z, std::mutex* mtx) {
		//assert(x >= 0 && x < width && y >= 0 && y < height);
		if (x < 0 || x >= width || y < 0 || y >= height) {
			return false;
		}
		if (z > (*this)(x, y)) {
			(*this)(x, y) = z;
			return true;
		}
		return false;
	}

	void resetBuffer() {
		long long size = static_cast<long long>(width) * height;
		for (long long i = 0; i < size; i++) {
			zbuffer.get()[i] = -INFINITY;
		}
	}

private:
	int width, height;
	std::shared_ptr<float> zbuffer;
};