//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		windows opengl interface
//
// $NoKeywords: $wingli
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinGLLegacyInterface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "WinEnvironment.h"
#include "OpenVRInterface.h"
#include "Engine.h"
#include "ConVar.h"

#include "OpenGLHeaders.h"

bool g_bARBMultisampleSupported = false;
int g_iARBMultisampleFormat = 0;



//****************//
//	Pixel Format  //
//****************//

PIXELFORMATDESCRIPTOR getPixelFormatDescriptor()
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

	// experimental, for ghost mode
	///pfd.cAlphaBits = 8;

	return pfd;
}

//**************************************//
//	check if an extension is supported  //
//**************************************//

bool wglIsExtensionSupported(const char *extension)
{
	const size_t extlen = strlen(extension);
	const char *supported = NULL;

	// try to use wglGetExtensionStringARB on current DC, if possible
	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString)
		supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());

	// if that failed, try standard opengl extensions string
	if (supported == NULL)
		supported = (char*)glGetString(GL_EXTENSIONS);

	// if that failed too, must be no extensions supported
	if (supported == NULL)
		return false;

	// begin examination at start of string, increment by 1 on false match
	for (const char* p = supported; ; p++)
	{
		// advance p up to the next possible match
		p = strstr(p, extension);

		if (p == NULL)
			return false; // no Match

		// make sure that match is at the start of the string, or that the previous char is a space, or else we could accidentally match "wglFunkywglExtension" with "wglExtension"
		// also, make sure that the following character is space or NULL, or else "wglExtensionTwo" might match "wglExtension"
		if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
			return true; // match
	}

	return false;
}

//************************************//
//	handle enabling of multisampling  //
//************************************//

bool initWinGLMultisample(HDC hDC, HINSTANCE hInstance, HWND hWnd, int factor)
{
	// check if the multisampling extension is available
	if (!wglIsExtensionSupported("WGL_ARB_multisample"))
		return false;

	// get our pixel format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

	if (!wglChoosePixelFormatARB)
		return false; // not supported extension

	int pixelFormat;
	bool valid;
	UINT numFormats;
	float fAttributes[] = {0,0};

	// attributes we want to test
	int iAttributes[] = { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
						  WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
						  WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
						  WGL_COLOR_BITS_ARB, 24,
						  WGL_ALPHA_BITS_ARB, 8,
						  WGL_DEPTH_BITS_ARB, 24,
						  WGL_STENCIL_BITS_ARB, 1,
						  WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
						  WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
						  WGL_SAMPLES_ARB, factor,
						  0,0
	};

	while (iAttributes[19] > 1)
	{
		// check if we can get a pixel format for the msaa value (iAttributes[19])
		valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);

		// if it succeeded, and numFormats is >= 1, then we're done
		if (valid && numFormats >= 1)
		{
			g_bARBMultisampleSupported = true;
			g_iARBMultisampleFormat = pixelFormat;
			return true;
		}

		// otherwise, divide by two to get the next possible msaa value, and try again (until < 2)
		iAttributes[19] = iAttributes[19] / 2;
	}

	return false;
}

FAKE_CONTEXT WinGLLegacyInterface::createAndMakeCurrentWGLContext(HWND hwnd, PIXELFORMATDESCRIPTOR pfdIn)
{
	FAKE_CONTEXT context;

	// get device context
	HDC tempHDC = GetDC(hwnd);
	context.hdc = tempHDC;

	// get pixel format
	int pixelFormat = ChoosePixelFormat(tempHDC, &pfdIn);
	debugLog("OpenGL: PixelFormat = %i\n",pixelFormat);

	// set pixel format
	BOOL result = SetPixelFormat(tempHDC, pixelFormat, &pfdIn);
	debugLog("OpenGL: SetPixelFormat() = %i\n",result);

	// create temp context and make current
	context.hglrc = wglCreateContext(tempHDC);
	wglMakeCurrent(tempHDC, context.hglrc);

	return context;
}



WinGLLegacyInterface::WinGLLegacyInterface(HWND hwnd) : OpenGLLegacyInterface()
{
	m_hwnd = hwnd;

	if (!checkGLHardwareAcceleration())
	{
		engine->showMessageErrorFatal("Fatal Engine Error", "No OpenGL hardware acceleration available!\nThe engine will quit now.");
		exit(0);
	}

	m_vr_liv = convar->getConVarByName("vr_liv");

	// get device context
	m_hdc = GetDC(m_hwnd);

	// get pixel format, use the MSAA one if it is supported
	PIXELFORMATDESCRIPTOR pfd = getPixelFormatDescriptor();
	int pixelFormat = 0;
	if (g_bARBMultisampleSupported)
		pixelFormat = g_iARBMultisampleFormat;
	else
		pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
	debugLog("OpenGL: PixelFormat = %i\n",pixelFormat);

	// set pixel format
	BOOL result = SetPixelFormat(m_hdc, pixelFormat, &pfd);
	debugLog("OpenGL: SetPixelFormat() = %i\n",result);

	// WINDOWS: HACKHACK: create temp context and make current
	m_hglrc = wglCreateContext(m_hdc);
	wglMakeCurrent(m_hdc, m_hglrc);

	if (!m_hglrc)
	{
		engine->showMessageErrorFatal("OpenGL Error", "Couldn't wglCreateContext()!\nThe engine will quit now.");
		engine->shutdown();
	}
}

WinGLLegacyInterface::~WinGLLegacyInterface()
{
	if (m_hdc != NULL)
		wglMakeCurrent(m_hdc, NULL); // deselect gl
	if (m_hglrc != NULL)
		wglDeleteContext(m_hglrc);	// delete gl
	if (m_hdc != NULL)
		DeleteDC(m_hdc);			// delete hdc
}

void WinGLLegacyInterface::endScene()
{
	OpenGLLegacyInterface::endScene();
	if (m_vr_liv->getBool() && openvr->isReady() && openvr->isLIVReady())
		wglSwapLayerBuffers(m_hdc, WGL_SWAP_MAIN_PLANE); // currently required for liv support to hook correctly, not feeling confident enough using it for non-VR
	else
		SwapBuffers(m_hdc);
}

void WinGLLegacyInterface::setVSync(bool vsync)
{
	typedef BOOL (__stdcall *PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (char*)glGetString( GL_EXTENSIONS );
	if (extensions == NULL || strstr(extensions, "WGL_EXT_swap_control") == 0)
	{
		debugLog("OpenGL: Can't set VSync, WGL_EXT_swap_control not supported!\n");
		return;
	}
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
		if( wglSwapIntervalEXT )
			wglSwapIntervalEXT((int)vsync);
		else
			debugLog("OpenGL: Can't set VSync, wglSwapIntervalEXT not supported!\n");
	}
}

bool WinGLLegacyInterface::checkGLHardwareAcceleration()
{
	HDC hdc = GetDC(((WinEnvironment*)env)->getHwnd());
	PIXELFORMATDESCRIPTOR pfd = getPixelFormatDescriptor();
	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
	ReleaseDC(((WinEnvironment*)env)->getHwnd(), hdc);
	return pixelFormat;
}

#endif

#endif
