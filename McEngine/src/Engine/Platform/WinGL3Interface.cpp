//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		windows opengl 3.x interface
//
// $NoKeywords: $wingl3i
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinGL3Interface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "Engine.h"
#include "WinEnvironment.h"

#include "OpenGLHeaders.h"

PIXELFORMATDESCRIPTOR WinGL3Interface::getPixelFormatDescriptor()
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_COMPOSITION;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	return pfd;
}

WinGL3Interface::WinGL3Interface(HWND hwnd) : OpenGL3Interface()
{
	m_hwnd = hwnd;
	m_hglrc = NULL;
	m_hdc = NULL;

	if (!checkGLHardwareAcceleration())
	{
		engine->showMessageErrorFatal("Fatal Engine Error", "No OpenGL hardware acceleration available!\nThe engine will quit now.");
		exit(0);
	}

	// get device context
	m_hdc = GetDC(m_hwnd);

	// get pixel format
	PIXELFORMATDESCRIPTOR pfd = getPixelFormatDescriptor();
	int pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
	debugLog("OpenGL: PixelFormat = %i\n", pixelFormat);

	// set pixel format
	BOOL result = SetPixelFormat(m_hdc, pixelFormat, &pfd);
	debugLog("OpenGL: SetPixelFormat() = %i\n", result);

	// WINDOWS: create temp context and make current
	HGLRC tempContext = wglCreateContext(m_hdc);
	wglMakeCurrent(m_hdc, tempContext);

	// so that we can get to wglCreateContextAttribsARB
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (wglCreateContextAttribs == NULL)
	{
		engine->showMessageErrorFatal("Fatal Engine Error", "Couldn't get wglCreateContextAttribsARB()!\nThe engine will quit now.");
		exit(0);
	}

	const int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,	3,
		WGL_CONTEXT_MINOR_VERSION_ARB,	0,
		WGL_CONTEXT_FLAGS_ARB,			WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};

	m_hglrc = wglCreateContextAttribs(m_hdc, 0, attribs);
	wglMakeCurrent(m_hdc, m_hglrc);
	wglDeleteContext(tempContext);

	if (!m_hglrc)
	{
		engine->showMessageErrorFatal("Fatal OpenGL Error", "Couldn't wglCreateContext()!\nThe engine will quit now.");
		exit(0);
	}
}

WinGL3Interface::~WinGL3Interface()
{
	if (m_hdc != NULL)
		wglMakeCurrent(m_hdc, NULL); // deselect gl
	if (m_hglrc != NULL)
		wglDeleteContext(m_hglrc);	// delete gl
	if (m_hdc != NULL)
		DeleteDC(m_hdc);			// delete hdc
}

void WinGL3Interface::endScene()
{
	OpenGL3Interface::endScene();
	SwapBuffers(m_hdc);
}

void WinGL3Interface::setVSync(bool vsync)
{
	typedef BOOL (__stdcall *PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT((int)vsync);
	else
		debugLog("OpenGL: Can't set VSync, wglSwapIntervalEXT not supported!\n");
}

bool WinGL3Interface::checkGLHardwareAcceleration()
{
	HDC hdc = GetDC(((WinEnvironment*)env)->getHwnd());
	PIXELFORMATDESCRIPTOR pfd = getPixelFormatDescriptor();
	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	ReleaseDC(((WinEnvironment*)env)->getHwnd(), hdc);
	return pixelFormat;
}

#endif

#endif
