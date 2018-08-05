//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		openvr controller wrapper
//
// $NoKeywords: $vrc
//===============================================================================//

#include "OpenVRController.h"

#include "Engine.h"
#include "ConVar.h"

ConVar steamvr_hapticpulse_bug_workaround("steamvr_hapticpulse_bug_workaround", false);

bool OpenVRController::STEAMVR_BUG_WORKAROUND_FLIPFLOP = false;

#if !defined(MCENGINE_FEATURE_OPENVR)

OpenVRController::OpenVRController() : InputDevice()
{
	m_role = OpenVRController::ROLE::ROLE_INVALID;
}

#endif

#ifdef MCENGINE_FEATURE_OPENVR

OpenVRController::OpenVRController(vr::IVRSystem *hmd, OpenVRController::ROLE role) : InputDevice()
{
	m_hmd = hmd;
	m_role = role;

	m_ulButtonPressed = 0;
	m_ulButtonTouched = 0;

	m_fLastTriggerHapticPulseTime = 0.0f;

	for (int i=0; i<vr::k_unControllerStateAxisCount; i++)
	{
		m_rAxis[i].x = 0.0f;
		m_rAxis[i].y = 0.0f;
	}
}

void OpenVRController::update(uint64_t buttonPressed, uint64_t buttonTouched, vr::VRControllerAxis_t axes[vr::k_unControllerStateAxisCount])
{
	m_ulButtonPressed = buttonPressed;
	m_ulButtonTouched = buttonTouched;

	for (int i=0; i<vr::k_unControllerStateAxisCount; i++)
	{
		m_rAxis[i] = axes[i];
	}

	if (steamvr_hapticpulse_bug_workaround.getBool())
	{
		// execute buffered TriggerHapticPulse events
		// we only execute the newest buffered event here, all others are discarded
		if ((!STEAMVR_BUG_WORKAROUND_FLIPFLOP && m_role == OpenVRController::ROLE::ROLE_RIGHTHAND) || (STEAMVR_BUG_WORKAROUND_FLIPFLOP && m_role == OpenVRController::ROLE::ROLE_LEFTHAND))
		{
			if (m_triggerHapticPulseBuffer.size() > 0)
			{
				TRIGGER_HAPTIC_PULSE_EVENT ev = m_triggerHapticPulseBuffer[m_triggerHapticPulseBuffer.size()-1];
				m_triggerHapticPulseBuffer.clear();

				triggerHapticPulse(ev.durationMicroSec, ev.button);
			}
		}
	}
}

void OpenVRController::updateMatrixPose(Matrix4 &deviceToAbsoluteTracking)
{
	m_matrix = deviceToAbsoluteTracking;

	m_vPos.x = m_matrix[12];
	m_vPos.y = m_matrix[13];
	m_vPos.z = m_matrix[14];

	m_vDir = (m_matrix * Vector3(0, 0, -1)).normalize();
	m_vUp = (m_matrix * Vector3(0, 1, 0)).normalize();
	m_vRight = (m_matrix * Vector3(1, 0, 0)).normalize();
}

#endif

void OpenVRController::updateDebug(float triggerValue)
{
#if defined(MCENGINE_FEATURE_OPENVR)

	m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Trigger - vr::EVRButtonId::k_EButton_Axis0].x = triggerValue;

#endif
}

void OpenVRController::updateMatrixPoseDebug(Vector3 pos, Vector3 forward, Vector3 up, Vector3 right)
{
	m_vPos = pos;

	m_vDir = forward;
	m_vUp = up;
	m_vRight = right;
}

void OpenVRController::triggerHapticPulse(unsigned short durationMicroSec, OpenVRController::BUTTON button)
{
	if (durationMicroSec == 0 || engine->getTime() < m_fLastTriggerHapticPulseTime) return;

	m_fLastTriggerHapticPulseTime = engine->getTime(); // default, override below for specific libraries as needed

#if defined(MCENGINE_FEATURE_OPENVR)

	if (m_hmd == NULL) return;

	// "After this call the application may not trigger another haptic pulse on this controller and axis combination for 5ms."
	m_fLastTriggerHapticPulseTime = engine->getTime() + 0.00515f;

	if (steamvr_hapticpulse_bug_workaround.getBool())
	{
		if ((STEAMVR_BUG_WORKAROUND_FLIPFLOP && m_role == OpenVRController::ROLE::ROLE_RIGHTHAND) || (!STEAMVR_BUG_WORKAROUND_FLIPFLOP && m_role == OpenVRController::ROLE::ROLE_LEFTHAND))
		{
			// not our turn, buffer the call and execute it in the next update()
			TRIGGER_HAPTIC_PULSE_EVENT ev;
			ev.durationMicroSec = durationMicroSec;
			ev.button = button;
			m_triggerHapticPulseBuffer.push_back(ev);
		}
		else // ok, it's our turn
			m_hmd->TriggerHapticPulse(m_hmd->GetTrackedDeviceIndexForControllerRole(roleIdToOpenVR(m_role)), buttonIdToOpenVR(button) - vr::EVRButtonId::k_EButton_Axis0, durationMicroSec);
	}
	else
		m_hmd->TriggerHapticPulse(m_hmd->GetTrackedDeviceIndexForControllerRole(roleIdToOpenVR(m_role)), buttonIdToOpenVR(button) - vr::EVRButtonId::k_EButton_Axis0, durationMicroSec);

#endif
}

bool OpenVRController::isButtonPressed(OpenVRController::BUTTON button)
{
#if defined(MCENGINE_FEATURE_OPENVR)

	return m_ulButtonPressed & vr::ButtonMaskFromId(buttonIdToOpenVR(button));

#else
	return false;
#endif
}

float OpenVRController::getTrigger()
{
#if defined(MCENGINE_FEATURE_OPENVR)

	return m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Trigger - vr::EVRButtonId::k_EButton_Axis0].x;

#else
	return 0.0f;
#endif
}

Vector2 OpenVRController::getTouchpad()
{
#if defined(MCENGINE_FEATURE_OPENVR)

	return Vector2(m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Touchpad - vr::EVRButtonId::k_EButton_Axis0].x, m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Touchpad - vr::EVRButtonId::k_EButton_Axis0].y);

#else
	return Vector2(0,0);
#endif
}

#ifdef MCENGINE_FEATURE_OPENVR

vr::EVRButtonId OpenVRController::buttonIdToOpenVR(OpenVRController::BUTTON buttonId)
{
	switch (buttonId)
	{
	case OpenVRController::BUTTON::BUTTON_SYSTEM:
		return vr::EVRButtonId::k_EButton_System;
	case OpenVRController::BUTTON::BUTTON_APPLICATIONMENU:
		return vr::EVRButtonId::k_EButton_ApplicationMenu;
	case OpenVRController::BUTTON::BUTTON_GRIP:
		return vr::EVRButtonId::k_EButton_Grip;
	case OpenVRController::BUTTON::BUTTON_DPAD_LEFT:
		return vr::EVRButtonId::k_EButton_DPad_Left;
	case OpenVRController::BUTTON::BUTTON_DPAD_UP:
		return vr::EVRButtonId::k_EButton_DPad_Up;
	case OpenVRController::BUTTON::BUTTON_DPAD_RIGHT:
		return vr::EVRButtonId::k_EButton_DPad_Right;
	case OpenVRController::BUTTON::BUTTON_DPAD_DOWN:
		return vr::EVRButtonId::k_EButton_DPad_Down;
	case OpenVRController::BUTTON::BUTTON_A:
		return vr::EVRButtonId::k_EButton_A;
	case OpenVRController::BUTTON::BUTTON_PROXIMITYSENSOR:
		return vr::EVRButtonId::k_EButton_ProximitySensor;
	case OpenVRController::BUTTON::BUTTON_AXIS0:
		return vr::EVRButtonId::k_EButton_Axis0;
	case OpenVRController::BUTTON::BUTTON_AXIS1:
		return vr::EVRButtonId::k_EButton_Axis1;
	case OpenVRController::BUTTON::BUTTON_AXIS2:
		return vr::EVRButtonId::k_EButton_Axis2;
	case OpenVRController::BUTTON::BUTTON_AXIS3:
		return vr::EVRButtonId::k_EButton_Axis3;
	case OpenVRController::BUTTON::BUTTON_AXIS4:
		return vr::EVRButtonId::k_EButton_Axis4;
	}

	return vr::EVRButtonId::k_EButton_Max;
}

vr::ETrackedControllerRole OpenVRController::roleIdToOpenVR(OpenVRController::ROLE roleId)
{
	switch (roleId)
	{
	case OpenVRController::ROLE::ROLE_INVALID:
		return vr::ETrackedControllerRole::TrackedControllerRole_Invalid;
	case OpenVRController::ROLE::ROLE_LEFTHAND:
		return vr::ETrackedControllerRole::TrackedControllerRole_LeftHand;
	case OpenVRController::ROLE::ROLE_RIGHTHAND:
		return vr::ETrackedControllerRole::TrackedControllerRole_RightHand;
	}

	return vr::ETrackedControllerRole::TrackedControllerRole_Invalid;
}

#endif
