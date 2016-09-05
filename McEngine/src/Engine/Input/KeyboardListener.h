//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		simple listener class for keyboard events
//
// $NoKeywords: $key
//===============================================================================//

#ifndef KEYBOARDLISTENER_H
#define KEYBOARDLISTENER_H

#include "KeyboardEvent.h"

class KeyboardListener
{
public:
	virtual ~KeyboardListener() {;}

	virtual void onKeyDown(KeyboardEvent &e) = 0;
	virtual void onKeyUp(KeyboardEvent &e) = 0;
	virtual void onChar(KeyboardEvent &e) = 0;
};

#endif
