//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		linux opengl interface
//
// $NoKeywords: $linuxgli
//===============================================================================//

#ifdef __linux__

#ifndef LINUXGLINTERFACE_H
#define LINUXGLINTERFACE_H

typedef unsigned char BYTE;

#include "OpenGLLegacyInterface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "OpenGLHeaders.h"

XVisualInfo *getVisualInfo(Display *display);

class LinuxGLLegacyInterface : public OpenGLLegacyInterface
{
public:
	LinuxGLLegacyInterface(Display *display, Window window);
	virtual ~LinuxGLLegacyInterface();

	// scene
	void endScene();

	// device settings
	void setVSync(bool vsync);

	// ILLEGAL:
	inline GLXContext getGLXContext() const {return m_glc;}

private:
	Display *m_display;
	Window m_window;
	GLXContext m_glc;
};

#endif

#endif

#endif
