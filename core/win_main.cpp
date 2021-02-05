#include <iostream>
#include "color.h"
#include "rasterizer.h"
#include "log/log.h"
#include "display/WindowDisplayer.h"

int WinMain(HINSTANCE hinstance, HINSTANCE prevHinstance, LPSTR cmd, int nCmd)
{
	Dwindow window(hinstance, prevHinstance, cmd, nCmd, 1024, 720, L"rasterizer");
	return window.display();
}
