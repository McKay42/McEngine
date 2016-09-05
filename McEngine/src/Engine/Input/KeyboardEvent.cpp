//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a wrapper for consumable keyboard events
//
// $NoKeywords: $key
//===============================================================================//

#include "KeyboardEvent.h"

KeyboardEvent::KeyboardEvent(KEYCODE keyCode)
{
	m_keyCode = keyCode;
	m_bConsumed = false;
}

void KeyboardEvent::consume()
{
	m_bConsumed = true;
}
