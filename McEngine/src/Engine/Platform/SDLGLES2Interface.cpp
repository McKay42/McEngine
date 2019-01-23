//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		nintendo switch opengl interface
//
// $NoKeywords: $nxgli
//===============================================================================//

#include "SDLGLES2Interface.h"

#if defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_OPENGLES)

#include "SDLEnvironment.h"

SDLGLES2Interface::SDLGLES2Interface(SDL_Window *window) : OpenGLES2Interface()
{
	m_window = window;
}

SDLGLES2Interface::~SDLGLES2Interface()
{
}

void SDLGLES2Interface::endScene()
{
	OpenGLES2Interface::endScene();
	SDL_GL_SwapWindow(m_window);
}

void SDLGLES2Interface::setVSync(bool vsync)
{
	SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

#endif
