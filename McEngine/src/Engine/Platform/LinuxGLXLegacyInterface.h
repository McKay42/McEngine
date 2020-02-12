//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		linux opengl interface
//
// $NoKeywords: $linuxgli
//===============================================================================//

#ifdef __linux__

#ifndef LINUXGLXLEGACYINTERFACE_H
#define LINUXGLXLEGACYINTERFACE_H

typedef unsigned char BYTE;

#include "OpenGLLegacyInterface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "OpenGLHeaders.h"

XVisualInfo *getVisualInfo(Display *display);

class LinuxGLXLegacyInterface : public OpenGLLegacyInterface
{
public:
	LinuxGLXLegacyInterface(Display *display, Window window);
	virtual ~LinuxGLXLegacyInterface();

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
