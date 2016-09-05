//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		top level gamepad interface
//
// $NoKeywords: $gamepad
//===============================================================================//

#include "Gamepad.h"

#include "Engine.h"

Gamepad::Gamepad() : InputDevice()
{
	m_fLeftStickDeadZoneX = m_fRightStickDeadZoneX = 0.28f;
	m_fLeftStickDeadZoneY = m_fRightStickDeadZoneY = 0.28f;
}

void Gamepad::addListener(GamepadListener *gamepadListener, bool insertOnTop)
{
	if (gamepadListener == NULL)
	{
		engine->showMessageError("Gamepad Error", "addListener( NULL )!");
		return;
	}

	if (insertOnTop)
		m_listeners.insert(m_listeners.begin(), gamepadListener);
	else
		m_listeners.push_back(gamepadListener);
}

void Gamepad::removeListener(GamepadListener *gamepadListener)
{
	for (int i=0; i<m_listeners.size(); i++)
	{
		if (m_listeners[i] == gamepadListener)
		{
			m_listeners.erase(m_listeners.begin() + i);
			i--;
		}
	}
}

void Gamepad::onButtonDown(GAMEPADBUTTON b)
{
	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onButtonDown(b);
	}
}

void Gamepad::onButtonUp(GAMEPADBUTTON b)
{
	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onButtonUp(b);
	}
}

void Gamepad::onConnected()
{
	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onConnected();
	}
}

void Gamepad::onDisconnected()
{
	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onDisconnected();
	}
}
