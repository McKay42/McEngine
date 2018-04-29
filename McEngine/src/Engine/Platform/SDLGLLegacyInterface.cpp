//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		sdl opengl interface
//
// $NoKeywords: $sdlgli
//===============================================================================//

#include "SDLGLLegacyInterface.h"

#if defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_OPENGL)

#include "Engine.h"

SDLGLLegacyInterface::SDLGLLegacyInterface(SDL_Window *window) : OpenGLLegacyInterface()
{
	m_window = window;
}

SDLGLLegacyInterface::~SDLGLLegacyInterface()
{
}

void SDLGLLegacyInterface::endScene()
{
	OpenGLLegacyInterface::endScene();
	SDL_GL_SwapWindow(m_window);
}

void SDLGLLegacyInterface::setVSync(bool vsync)
{
	SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

#endif
