//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		macOS opengl interface
//
// $NoKeywords: $macgli
//===============================================================================//

#ifdef __APPLE__

#include "MacOSGLLegacyInterface.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "MacOSEnvironment.h"
#include "main_OSX_cpp.h"
#include "Engine.h"

#include "OpenGLHeaders.h"

MacOSGLLegacyInterface::MacOSGLLegacyInterface() : OpenGLLegacyInterface()
{
}

MacOSGLLegacyInterface::~MacOSGLLegacyInterface()
{
}

void MacOSGLLegacyInterface::endScene()
{
	glFlush();
}

void MacOSGLLegacyInterface::setVSync(bool vsync)
{
	((MacOSEnvironment *)(env))->getWrapper()->setVSync(vsync);
}

#endif

#endif
