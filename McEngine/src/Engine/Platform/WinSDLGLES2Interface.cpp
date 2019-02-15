//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		windows sdl opengl es 2.x interface
//
// $NoKeywords: $winsdlgles2i
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinSDLGLES2Interface.h"

#if defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_OPENGLES)

#include "Engine.h"

#include "OpenGLHeaders.h"

WinSDLGLES2Interface::WinSDLGLES2Interface(SDL_Window *window) : SDLGLES2Interface(window)
{
	// check GLEW
	glewExperimental = GL_TRUE; // TODO: upgrade to glew >= 2.0.0 to fix this (would cause crash in e.g. glGenVertexArrays() without it)
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		debugLog("glewInit() Error: %s\n", glewGetErrorString(err));
		engine->showMessageErrorFatal("OpenGL Error", "Couldn't glewInit()!\nThe engine will exit now.");
		engine->shutdown();
		return;
	}
}

WinSDLGLES2Interface::~WinSDLGLES2Interface()
{
}

#endif

#endif
