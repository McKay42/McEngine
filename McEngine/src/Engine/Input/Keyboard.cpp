//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		state class and listeners (FIFO)
//
// $NoKeywords: $key
//===============================================================================//

#include "Keyboard.h"

#include "Engine.h"

Keyboard::Keyboard() : InputDevice()
{
	m_bControlDown = false;
	m_bAltDown = false;
	m_bShiftDown = false;
	m_bSuperDown = false;
}

void Keyboard::addListener(KeyboardListener *keyboardListener, bool insertOnTop)
{
	if (keyboardListener == NULL)
	{
		engine->showMessageError("Keyboard Error", "addListener(NULL)!");
		return;
	}

	if (insertOnTop)
		m_listeners.insert(m_listeners.begin(), keyboardListener);
	else
		m_listeners.push_back(keyboardListener);
}

void Keyboard::removeListener(KeyboardListener *keyboardListener)
{
	for (int i=0; i<m_listeners.size(); i++)
	{
		if (m_listeners[i] == keyboardListener)
		{
			m_listeners.erase(m_listeners.begin() + i);
			i--;
		}
	}
}

void Keyboard::reset()
{
	m_bControlDown = false;
	m_bAltDown = false;
	m_bShiftDown = false;
	m_bSuperDown = false;
}

void Keyboard::onKeyDown(KEYCODE keyCode)
{
	switch (keyCode)
	{
	case KEY_CONTROL:
		m_bControlDown = true;
		break;
	case KEY_ALT:
		m_bAltDown = true;
		break;
	case KEY_SHIFT:
		m_bShiftDown = true;
		break;
	case KEY_SUPER:
		m_bSuperDown = true;
		break;
	}

	KeyboardEvent e(keyCode);

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onKeyDown(e);
		if (e.isConsumed())
			break;
	}
}

void Keyboard::onKeyUp(KEYCODE keyCode)
{
	switch (keyCode)
	{
	case KEY_CONTROL:
		m_bControlDown = false;
		break;
	case KEY_ALT:
		m_bAltDown = false;
		break;
	case KEY_SHIFT:
		m_bShiftDown = false;
		break;
	case KEY_SUPER:
		m_bSuperDown = false;
		break;
	}

	KeyboardEvent e(keyCode);

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onKeyUp(e);
		if (e.isConsumed())
			break;
	}
}

void Keyboard::onChar(KEYCODE charCode)
{
	KeyboardEvent e(charCode);

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onChar(e);
		if (e.isConsumed())
			break;
	}
}
