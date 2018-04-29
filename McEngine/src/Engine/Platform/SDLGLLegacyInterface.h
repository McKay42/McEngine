//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		sdl opengl interface
//
// $NoKeywords: $sdlgli
//===============================================================================//

#ifndef SDLGLLEGACYINTERFACE_H
#define SDLGLLEGACYINTERFACE_H

#include "OpenGLLegacyInterface.h"

#if defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_OPENGL)

#include "SDL.h"

class SDLGLLegacyInterface : public OpenGLLegacyInterface
{
public:
	SDLGLLegacyInterface(SDL_Window *window);
	virtual ~SDLGLLegacyInterface();

	// scene
	void endScene();

	// device settings
	void setVSync(bool vsync);

private:
	SDL_Window *m_window;
};

#endif

#endif
