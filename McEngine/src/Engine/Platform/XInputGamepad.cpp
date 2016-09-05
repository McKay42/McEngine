//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		xinput gamepad wrapper
//
// $NoKeywords: $xi
//===============================================================================//

#include "XInputGamepad.h"

#include "Engine.h"

XInputGamepad::XInputGamepad() : Gamepad()
{
	m_iPort = -1;

	m_fLeftTrigger = 0.0f;
	m_fRightTrigger = 0.0f;

	m_bDpadUpPressed = false;
	m_bDpadDownPressed = false;
	m_bDpadLeftPressed = false;
	m_bDpadRightPressed = false;
	m_bStartPressed = false;
	m_bBackPressed = false;
	m_bLeftThumbPressed = false;
	m_bRightThumbPressed = false;
	m_bLeftShoulderPressed = false;
	m_bRightShoulderPressed = false;
	m_bAPressed = false;
	m_bBPressed = false;
	m_bXPressed = false;
	m_bYPressed = false;
}

void XInputGamepad::update()
{
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#ifdef MCENGINE_FEATURE_GAMEPAD

	if (!isConnected())
		updateConnection();
	else
	{
		memset(&m_state, 0, sizeof(XINPUT_STATE));
		if (XInputGetState(m_iPort, &m_state) != 0L)
		{
			// controller has been disconnected
			debugLog(0xffffff00, "XINPUTGAMEPAD: #%i disconnected\n", m_iPort);
			onDisconnected();
			m_iPort = -1;
			return;
		}
		else
		{
			// get fresh values from the controller

			// left stick
			float normLX = fmaxf(-1, (float) m_state.Gamepad.sThumbLX / 32767);
			float normLY = fmaxf(-1, (float) m_state.Gamepad.sThumbLY / 32767);

			m_vLeftStick.x = (std::abs(normLX) < m_fLeftStickDeadZoneX ? 0 : (std::abs(normLX) - m_fLeftStickDeadZoneX) * (normLX / std::abs(normLX)));
			m_vLeftStick.y = (std::abs(normLY) < m_fLeftStickDeadZoneY ? 0 : (std::abs(normLY) - m_fLeftStickDeadZoneY) * (normLY / std::abs(normLY)));

			if (m_fLeftStickDeadZoneX > 0)
				m_vLeftStick.x *= 1 / (1 - m_fLeftStickDeadZoneX);
			if (m_fLeftStickDeadZoneY > 0)
				m_vLeftStick.y *= 1 / (1 - m_fLeftStickDeadZoneY);


			// right stick
			float normRX = fmaxf(-1, (float) m_state.Gamepad.sThumbRX / 32767);
			float normRY = fmaxf(-1, (float) m_state.Gamepad.sThumbRY / 32767);

			m_vRightStick.x = (std::abs(normRX) < m_fRightStickDeadZoneX ? 0 : (std::abs(normRX) - m_fRightStickDeadZoneX) * (normRX / std::abs(normRX)));
			m_vRightStick.y = (std::abs(normRY) < m_fRightStickDeadZoneY ? 0 : (std::abs(normRY) - m_fRightStickDeadZoneY) * (normRY / std::abs(normRY)));

			if (m_fRightStickDeadZoneX > 0)
				m_vRightStick.x *= 1 / (1 - m_fRightStickDeadZoneX);
			if (m_fRightStickDeadZoneY > 0)
				m_vRightStick.y *= 1 / (1 - m_fRightStickDeadZoneY);


			// triggers
			m_fLeftTrigger = (float) m_state.Gamepad.bLeftTrigger / 255;
			m_fRightTrigger = (float) m_state.Gamepad.bRightTrigger / 255;


			// and all buttons
			updateButtonStates();
		}
	}

#endif
#endif
}

void XInputGamepad::updateConnection()
{
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#ifdef MCENGINE_FEATURE_GAMEPAD

	int controllerId = -1;

	// get first controller
	for (DWORD i=0; i<XUSER_MAX_COUNT && controllerId == -1; i++)
	{
		XINPUT_STATE state;
		memset(&state, 0, sizeof(XINPUT_STATE));

		if (XInputGetState(i, &state) == 0L)
			controllerId = i;
	}

	if (controllerId != m_iPort && controllerId != -1)
	{
		debugLog(0xffffff00, "XINPUTGAMEPAD: #%i connected\n", controllerId);
		onConnected();
	}

	m_iPort = controllerId;

#endif
#endif
}

void XInputGamepad::updateButtonStates()
{
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#ifdef MCENGINE_FEATURE_GAMEPAD

	const bool dpadUpPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
	const bool dpadDownPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
	const bool dpadLeftPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
	const bool dpadRightPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
	const bool startPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
	const bool backPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
	const bool leftThumbPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
	const bool rightThumbPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
	const bool leftShoulderPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
	const bool rightShoulderPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
	const bool aPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
	const bool bPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
	const bool xPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
	const bool yPressed = (m_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;

	checkHandleButtonChangeEvents(m_bDpadUpPressed, dpadUpPressed, GAMEPAD_DPAD_UP);
	checkHandleButtonChangeEvents(m_bDpadDownPressed, dpadDownPressed, GAMEPAD_DPAD_DOWN);
	checkHandleButtonChangeEvents(m_bDpadLeftPressed, dpadLeftPressed, GAMEPAD_DPAD_LEFT);
	checkHandleButtonChangeEvents(m_bDpadRightPressed, dpadRightPressed, GAMEPAD_DPAD_RIGHT);
	checkHandleButtonChangeEvents(m_bStartPressed, startPressed, GAMEPAD_START);
	checkHandleButtonChangeEvents(m_bBackPressed, backPressed, GAMEPAD_BACK);
	checkHandleButtonChangeEvents(m_bLeftThumbPressed, leftThumbPressed, GAMEPAD_LEFT_THUMB);
	checkHandleButtonChangeEvents(m_bRightThumbPressed, rightThumbPressed, GAMEPAD_RIGHT_THUMB);
	checkHandleButtonChangeEvents(m_bRightShoulderPressed, rightShoulderPressed, GAMEPAD_RIGHT_SHOULDER);
	checkHandleButtonChangeEvents(m_bLeftShoulderPressed, leftShoulderPressed, GAMEPAD_RIGHT_SHOULDER);
	checkHandleButtonChangeEvents(m_bAPressed, aPressed, GAMEPAD_A);
	checkHandleButtonChangeEvents(m_bBPressed, bPressed, GAMEPAD_B);
	checkHandleButtonChangeEvents(m_bXPressed, xPressed, GAMEPAD_X);
	checkHandleButtonChangeEvents(m_bYPressed, yPressed, GAMEPAD_Y);

#endif
#endif
}

void XInputGamepad::checkHandleButtonChangeEvents(bool &previous, bool current, GAMEPADBUTTON b)
{
	if (previous != current)
	{
		previous = current;
		if (current)
			onButtonDown(b);
		else
			onButtonUp(b);
	}
}

bool XInputGamepad::isButtonPressed(GAMEPADBUTTON button)
{
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#ifdef MCENGINE_FEATURE_GAMEPAD

	switch (button)
	{
	// DPAD
	case GAMEPAD_DPAD_UP:
		return m_bDpadUpPressed;
		break;
	case GAMEPAD_DPAD_DOWN:
		return m_bDpadDownPressed;
		break;
	case GAMEPAD_DPAD_LEFT:
		return m_bDpadLeftPressed;
		break;
	case GAMEPAD_DPAD_RIGHT:
		return m_bDpadRightPressed;
		break;

	// START/SELECT
	case GAMEPAD_START:
		return m_bStartPressed;
		break;
	case GAMEPAD_BACK:
		return m_bBackPressed;
		break;

	// STICKS
	case GAMEPAD_LEFT_THUMB:
		return m_bLeftThumbPressed;
		break;
	case GAMEPAD_RIGHT_THUMB:
		return m_bRightThumbPressed;
		break;

	// SHOULDER
	case GAMEPAD_LEFT_SHOULDER:
		return m_bLeftShoulderPressed;
		break;
	case GAMEPAD_RIGHT_SHOULDER:
		return m_bRightShoulderPressed;
		break;

	// A/B/X/Y
	case GAMEPAD_A:
		return m_bAPressed;
		break;
	case GAMEPAD_B:
		return m_bBPressed;
		break;
	case GAMEPAD_X:
		return m_bXPressed;
		break;
	case GAMEPAD_Y:
		return m_bYPressed;
		break;

	default:
		return false;
	}

#else
	return false;
#endif
#else
	return false;
#endif
}
