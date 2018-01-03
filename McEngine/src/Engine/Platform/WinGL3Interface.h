//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		windows opengl 3.x interface
//
// $NoKeywords: $wingl3i
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifndef WINGL3INTERFACE_H
#define WINGL3INTERFACE_H

#include "OpenGL3Interface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include <Windows.h>

class WinGL3Interface : public OpenGL3Interface
{
public:
	static PIXELFORMATDESCRIPTOR getPixelFormatDescriptor();

public:
	WinGL3Interface(HWND hwnd);
	virtual ~WinGL3Interface();

	// scene
	void endScene();

	// device settings
	void setVSync(bool vsync);

	// ILLEGAL:
	bool checkGLHardwareAcceleration();
	inline HGLRC getGLContext() const {return m_hglrc;}
	inline HDC getHDC() const {return m_hdc;}

private:
	// device context
	HWND m_hwnd;
	HGLRC m_hglrc;
	HDC m_hdc;
};

#endif

#endif

#endif
