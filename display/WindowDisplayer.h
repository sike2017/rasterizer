#pragma once
#include <Windows.h>
#include "Painter.h"
#include "log/log.h"

extern Painter painter;
// do not using this namespace in your program!
namespace display
{
	extern int screen_keys[512];
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
}

class WindowDisplayer
{
public:
	WindowDisplayer(HINSTANCE thInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow, int twidth, int theight, const wchar_t* twindowName);
	~WindowDisplayer() {}

	int display();
	void setPixel(int x, int y, RGBA color);
	void update();
	
	int width() const;
	int height() const;

protected:
	int colorIndex;

	int _width, _height;

	enum class RENDER_STATUS
	{
		CALL_NEXTTIME,
		CALL_STOP,
		CALL_STOP_SAVE_IMAGE
	};

	RENDER_STATUS renderStatus;

	virtual RENDER_STATUS render() {
		rlog.print("virtual render\n");

		RGBA color0(0, 162, 232, 0);
		RGBA color1(112, 146, 190, 0);

		int width = 400;
		int height = 300;
		int size = width * height;
		for (int i = 0; i < size; i++)
		{
			int x = i % width;
			int y = i / width;
			if (!colorIndex)
			{
				setPixel(x, y, color0);
			}
			else {
				setPixel(x, y, color1);
			}
		}

		colorIndex = !colorIndex;

		update();


		return RENDER_STATUS::CALL_NEXTTIME;
	}
	virtual void keyboardEvent(int* screen_keys) {}

private:
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR szCmdLine;
	int iCmdShow;
	std::wstring windowName;
	
	RenderBitmap rb;

	int InitWindow();
};