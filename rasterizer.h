#pragma once
#include "display/WindowDisplayer.h"
#include "color.h"

class Dwindow : public WindowDisplayer {
public:
	Dwindow(HINSTANCE thInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow, int _width, int _height, const wchar_t* twindowName)
		: WindowDisplayer(thInstance, hPrevInstance, szCmdLine, iCmdShow, _width, _height, twindowName), width(_width), height(_height) {}
	~Dwindow() {}

protected:
	virtual RENDER_STATUS render() override {
		sky();
		update();
		return RENDER_STATUS::CALL_STOP;
	}

private:
	void sky() {
		Color topColor = Color(0.5, 0.7, 1.0);
		Color col;
		uint8_t ur, ug, ub;
		for (int y = height - 1; y >= 0 ; y--) {
			for (int x = 0; x < width; x++) {
				float r = float(x) / float(width);
				float g = float(y) / float(height);
				float b = 0.2;
				ur = 255.99 * r;
				ug = 255.99 * g;
				ub = 255.99 * b;
				setPixel(x, y, RGBA(ur, ug, ub, 255));
			}
		}
	}

	int width, height;
};
