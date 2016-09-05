//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		state class and listeners (FIFO)
//
// $NoKeywords: $key
//===============================================================================//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "InputDevice.h"
#include "KeyboardKeys.h"
#include "KeyboardEvent.h"
#include "KeyboardListener.h"

class Keyboard : public InputDevice
{
public:
	Keyboard();
	virtual ~Keyboard() {;}

	void addListener(KeyboardListener *keyboardListener, bool insertOnTop = false);
	void removeListener(KeyboardListener *keyboardListener);
	void reset();

	virtual void onKeyDown(KEYCODE keyCode);
	virtual void onKeyUp(KEYCODE keyCode);
	virtual void onChar(KEYCODE charCode);

	inline bool isControlDown() const {return m_bControlDown;}
	inline bool isAltDown() const {return m_bAltDown;}
	inline bool isShiftDown() const {return m_bShiftDown;}
	inline bool isSuperDown() const {return m_bSuperDown;}

private:
	bool m_bControlDown;
	bool m_bAltDown;
	bool m_bShiftDown;
	bool m_bSuperDown;

	std::vector<KeyboardListener*> m_listeners;
};

#endif
