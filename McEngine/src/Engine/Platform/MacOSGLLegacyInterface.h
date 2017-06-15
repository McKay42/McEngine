//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		macOS opengl interface
//
// $NoKeywords: $macgli
//===============================================================================//

#ifdef __APPLE__

#ifndef MACOSGLLEGACYINTERFACE_H
#define MACOSGLLEGACYINTERFACE_H

#include "OpenGLLegacyInterface.h"

class MacOSGLLegacyInterface : public OpenGLLegacyInterface
{
public:
	MacOSGLLegacyInterface();
	virtual ~MacOSGLLegacyInterface();

	// scene
	void endScene();

	// device settings
	void setVSync(bool vsync);

private:
};

#endif

#endif
