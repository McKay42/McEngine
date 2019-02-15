//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		nintendo switch opengl interface
//
// $NoKeywords: $nxgli
//===============================================================================//

#ifndef SDLGLES2INTERFACE_H
#define SDLGLES2INTERFACE_H

#include "OpenGLES2Interface.h"

#if defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_OPENGLES)

#include "SDL.h"

class SDLGLES2Interface : public OpenGLES2Interface
{
public:
	SDLGLES2Interface(SDL_Window *window);
	virtual ~SDLGLES2Interface();

	// scene
	virtual void endScene();

	// device settings
	virtual void setVSync(bool vsync);

private:
	SDL_Window *m_window;
};

#endif

#endif
