//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		linux opengl interface
//
// $NoKeywords: $linuxgli
//===============================================================================//

#ifdef __linux__

#include "LinuxGLLegacyInterface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "LinuxEnvironment.h"
#include "Engine.h"
#include "ConVar.h"

XVisualInfo *getVisualInfo(Display *display)
{
	GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 1, GLX_DOUBLEBUFFER, None};
	return glXChooseVisual(display, 0, att);
}

LinuxGLLegacyInterface::LinuxGLLegacyInterface(Display *display, Window window) : OpenGLLegacyInterface()
{
	m_display = display;
	m_window = window;

	// create opengl context and make current
	m_glc = glXCreateContext(m_display, getVisualInfo(m_display), NULL, GL_TRUE);
	if (m_glc == NULL)
	{
		engine->showMessageError("OpenGL Error", "Couldn't glXCreateContext()!\nThe engine will quit now.");
		return;
	}
	glXMakeCurrent(m_display, m_window, m_glc);
}

LinuxGLLegacyInterface::~LinuxGLLegacyInterface()
{
	glXMakeCurrent(m_display, None, NULL);
	glXDestroyContext(m_display, m_glc);
}

void LinuxGLLegacyInterface::endScene()
{
	OpenGLLegacyInterface::endScene();
	glXSwapBuffers(m_display, m_window);
}

void LinuxGLLegacyInterface::setVSync(bool vsync)
{
	typedef void (*PFNWGLSWAPINTERVALPROC)(Display*, GLXDrawable, int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (char*)glXQueryExtensionsString(m_display, 0);
	if (strstr(extensions, "GLX_EXT_swap_control") == 0)
	{
		debugLog("OpenGL: Can't set VSync, GLX_EXT_swap_control not supported!\n");
		return;
	}
	else
	{
		const char *funcName = "glXSwapIntervalEXT";
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)glXGetProcAddress((unsigned char*)funcName);
		if( wglSwapIntervalEXT )
			wglSwapIntervalEXT(m_display, glXGetCurrentDrawable(), vsync ? 1 : 0);
	}
}

#endif

#endif
