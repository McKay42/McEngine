//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		top level gamepad interface
//
// $NoKeywords: $gamepad
//===============================================================================//

#ifndef GAMEPAD_H
#define GAMEPAD_H

#include "InputDevice.h"
#include "GamepadButtons.h"
#include "GamepadListener.h"

class Gamepad : public InputDevice
{
public:
	Gamepad();
	virtual ~Gamepad() {;}

	void addListener(GamepadListener *gamepadListener, bool insertOnTop = false);
	void removeListener(GamepadListener *gamepadListener);

	void onButtonDown(GAMEPADBUTTON b);
	void onButtonUp(GAMEPADBUTTON b);

	void onConnected();
	void onDisconnected();

	virtual Vector2 getLeftStick() = 0;
	virtual Vector2 getRightStick() = 0;

	virtual float getLeftTrigger() = 0;
	virtual float getRightTrigger() = 0;

	virtual bool isButtonPressed(GAMEPADBUTTON button) = 0;

	virtual bool isConnected() = 0;
	virtual int getPort() = 0;

	void setLeftStickDeadZone(float leftStickDeadZoneX, float leftStickDeadZoneY) {m_fLeftStickDeadZoneX = leftStickDeadZoneX; m_fLeftStickDeadZoneY = leftStickDeadZoneY;}
	void setLeftStickDeadZoneX(float leftStickDeadZoneX) {m_fLeftStickDeadZoneX = leftStickDeadZoneX;}
	void setLeftStickDeadZoneY(float leftStickDeadZoneY) {m_fLeftStickDeadZoneY = leftStickDeadZoneY;}
	void setRightStickDeadZone(float rightStickDeadZoneX, float rightStickDeadZoneY) {m_fRightStickDeadZoneX = rightStickDeadZoneX; m_fRightStickDeadZoneY = rightStickDeadZoneY;}
	void setRightStickDeadZoneX(float rightStickDeadZoneX) {m_fRightStickDeadZoneX = rightStickDeadZoneX;}
	void setRightStickDeadZoneY(float rightStickDeadZoneY) {m_fRightStickDeadZoneY = rightStickDeadZoneY;}

protected:
	float m_fLeftStickDeadZoneX;
	float m_fLeftStickDeadZoneY;
	float m_fRightStickDeadZoneX;
	float m_fRightStickDeadZoneY;

private:
	std::vector<GamepadListener*> m_listeners;
};

#endif
