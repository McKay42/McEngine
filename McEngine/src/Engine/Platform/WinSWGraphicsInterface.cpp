//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		windows software rasterizer graphics interface
//
// $NoKeywords: $winswi
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinSWGraphicsInterface.h"
#include "WinGLLegacyInterface.h"
#include "WinEnvironment.h"
#include "Engine.h"

WinSWGraphicsInterface::WinSWGraphicsInterface(HWND hwnd) : SWGraphicsInterface()
{
	m_hwnd = hwnd;

	// get device context
	m_hdc = GetDC(m_hwnd);
}

WinSWGraphicsInterface::~WinSWGraphicsInterface()
{
	if (m_hdc != NULL)
		DeleteDC(m_hdc); // delete hdc
}

void WinSWGraphicsInterface::endScene()
{
	SWGraphicsInterface::endScene();

	// blit backBuffer to hdc
	SWGraphicsInterface::PIXEL *backBuffer = getBackBuffer();
	if (backBuffer != NULL)
	{
		Vector2 rendererResolution = getResolution();

		BITMAPINFO bminfo = {};
		bminfo.bmiHeader.biSize = sizeof(BITMAPINFO);
		bminfo.bmiHeader.biWidth = (int)rendererResolution.x;
		bminfo.bmiHeader.biHeight = -(int)rendererResolution.y; // invert
		bminfo.bmiHeader.biBitCount = 32;
		bminfo.bmiHeader.biCompression = BI_RGB;
		bminfo.bmiHeader.biPlanes = 1;
		bminfo.bmiHeader.biSizeImage = 0;
		bminfo.bmiHeader.biXPelsPerMeter = 0;
		bminfo.bmiHeader.biYPelsPerMeter = 0;
		bminfo.bmiHeader.biClrUsed = 0;
		bminfo.bmiHeader.biClrImportant = 0;

		StretchDIBits(m_hdc, 0, 0, (int)rendererResolution.x, (int)rendererResolution.y, 0, 0, (int)rendererResolution.x, (int)rendererResolution.y, backBuffer, &bminfo, DIB_RGB_COLORS, SRCCOPY);
	}
}

void WinSWGraphicsInterface::setVSync(bool vsync)
{
}

#endif
