//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		simple listener class for gamepad events
//
// $NoKeywords: $gamepad
//===============================================================================//

#ifndef GAMEPADLISTENER_H
#define GAMEPADLISTENER_H

#include "GamepadEvent.h"

class GamepadListener
{
public:
	virtual ~GamepadListener() {;}

	virtual void onButtonDown(GAMEPADBUTTON b) = 0;
	virtual void onButtonUp(GAMEPADBUTTON b) = 0;

	virtual void onConnected() = 0;
	virtual void onDisconnected() = 0;
};

#endif
