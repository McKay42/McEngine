//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		top level interface for native OS calls
//
// $NoKeywords: $env
//===============================================================================//

#include "Environment.h"

#include "ConVar.h"

Environment::Environment()
{
	m_bFullscreenWindowedBorderless = false;
}

void Environment::setFullscreenWindowedBorderless(bool fullscreenWindowedBorderless)
{
	m_bFullscreenWindowedBorderless = fullscreenWindowedBorderless;

	if (env->isFullscreen())
	{
		env->disableFullscreen();
		env->enableFullscreen();
	}
}



//***************************//
//	Environment ConCommands  //
//***************************//

void _fullscreen_windowed_borderless(UString oldValue, UString newValue)
{
	env->setFullscreenWindowedBorderless(newValue.toFloat() > 0.0f);
}

ConVar __fullscreen_windowed_borderless("fullscreen_windowed_borderless", false, _fullscreen_windowed_borderless);
