//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		sdl implementation of main_OSX
//
// $NoKeywords: $macsdl
//===============================================================================//

#ifdef __APPLE__

#include "EngineFeatures.h"
#include "main_OSX_cpp.h"

#ifdef MCENGINE_FEATURE_SDL

#include "MacOSSDLEnvironment.h"

extern int mainSDL(int argc, char *argv[], SDLEnvironment *customSDLEnvironment);

int main(int argc, char* argv[])
{
	mainSDL(argc, argv, new MacOSSDLEnvironment());
}

#endif

#endif
