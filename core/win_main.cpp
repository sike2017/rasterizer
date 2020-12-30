#include <iostream>
#include "color.h"
#include "rasterizer.h"
#include "log/log.h"
#include "display/WindowDisplayer.h"

int WinMain(HINSTANCE hinstance, HINSTANCE prevHinstance, LPSTR cmd, int nCmd)
{
	rlog << "Hello CMake.\n" << rendl;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	Dwindow window(hInstance, prevHinstance, cmd, nCmd, 1024, 720, L"rasterizer");
	return window.display();
}
