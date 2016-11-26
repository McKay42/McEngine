//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		openvr controller wrapper
//
// $NoKeywords: $vrc
//===============================================================================//

#include "OpenVRController.h"

#include "Engine.h"
#include "ConVar.h"

#ifndef MCENGINE_FEATURE_OPENVR

OpenVRController::OpenVRController() : InputDevice()
{
	m_role = OpenVRController::ROLE::ROLE_INVALID;
}

#else

OpenVRController::OpenVRController(vr::IVRSystem *hmd, OpenVRController::ROLE role) : InputDevice()
{
	m_hmd = hmd;
	m_role = role;

	m_ulButtonPressed = 0;
	m_ulButtonTouched = 0;

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

void OpenVRController::triggerHapticPulse(unsigned short durationMicroSec, OpenVRController::BUTTON button)
{
#ifdef MCENGINE_FEATURE_OPENVR

	if (m_hmd == NULL) return;

	m_hmd->TriggerHapticPulse(m_hmd->GetTrackedDeviceIndexForControllerRole(roleIdToOpenVR(m_role)), buttonIdToOpenVR(button), durationMicroSec);

#endif
}

bool OpenVRController::isButtonPressed(OpenVRController::BUTTON button)
{
#ifdef MCENGINE_FEATURE_OPENVR

	return m_ulButtonPressed & vr::ButtonMaskFromId(buttonIdToOpenVR(button));

#else
	return false;
#endif
}

float OpenVRController::getTrigger()
{
#ifdef MCENGINE_FEATURE_OPENVR

	return m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Trigger - vr::EVRButtonId::k_EButton_Axis0].x;

#else
	return 0.0f;
#endif
}

Vector2 OpenVRController::getTouchpad()
{
#ifdef MCENGINE_FEATURE_OPENVR

	return Vector2(m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Touchpad - vr::EVRButtonId::k_EButton_Axis0].x, m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Touchpad - vr::EVRButtonId::k_EButton_Axis0].y);

#else
	return Vector2(0,0);
#endif
}

#ifdef MCENGINE_FEATURE_OPENVR

vr::EVRButtonId OpenVRController::buttonIdToOpenVR(OpenVRController::BUTTON buttonId)
{
	vr::EVRButtonId openvrButtonId = vr::EVRButtonId::k_EButton_Max;

	switch (buttonId)
	{
	case OpenVRController::BUTTON::BUTTON_SYSTEM:
		openvrButtonId = vr::EVRButtonId::k_EButton_System;
		break;
	case OpenVRController::BUTTON::BUTTON_APPLICATIONMENU:
		openvrButtonId = vr::EVRButtonId::k_EButton_ApplicationMenu;
		break;
	case OpenVRController::BUTTON::BUTTON_GRIP:
		openvrButtonId = vr::EVRButtonId::k_EButton_Grip;
		break;
	case OpenVRController::BUTTON::BUTTON_DPAD_LEFT:
		openvrButtonId = vr::EVRButtonId::k_EButton_DPad_Left;
		break;
	case OpenVRController::BUTTON::BUTTON_DPAD_UP:
		openvrButtonId = vr::EVRButtonId::k_EButton_DPad_Up;
		break;
	case OpenVRController::BUTTON::BUTTON_DPAD_RIGHT:
		openvrButtonId = vr::EVRButtonId::k_EButton_DPad_Right;
		break;
	case OpenVRController::BUTTON::BUTTON_DPAD_DOWN:
		openvrButtonId = vr::EVRButtonId::k_EButton_DPad_Down;
		break;
	case OpenVRController::BUTTON::BUTTON_A:
		openvrButtonId = vr::EVRButtonId::k_EButton_A;
		break;
	case OpenVRController::BUTTON::BUTTON_PROXIMITYSENSOR:
		openvrButtonId = vr::EVRButtonId::k_EButton_ProximitySensor;
		break;
	case OpenVRController::BUTTON::BUTTON_AXIS0:
		openvrButtonId = vr::EVRButtonId::k_EButton_Axis0;
		break;
	case OpenVRController::BUTTON::BUTTON_AXIS1:
		openvrButtonId = vr::EVRButtonId::k_EButton_Axis1;
		break;
	case OpenVRController::BUTTON::BUTTON_AXIS2:
		openvrButtonId = vr::EVRButtonId::k_EButton_Axis2;
		break;
	case OpenVRController::BUTTON::BUTTON_AXIS3:
		openvrButtonId = vr::EVRButtonId::k_EButton_Axis3;
		break;
	case OpenVRController::BUTTON::BUTTON_AXIS4:
		openvrButtonId = vr::EVRButtonId::k_EButton_Axis4;
		break;
	}

	return openvrButtonId;
}

vr::ETrackedControllerRole OpenVRController::roleIdToOpenVR(OpenVRController::ROLE roleId)
{
	vr::ETrackedControllerRole openvrRoleId = vr::ETrackedControllerRole::TrackedControllerRole_Invalid;

	switch (roleId)
	{
	case OpenVRController::ROLE::ROLE_INVALID:
		openvrRoleId = vr::ETrackedControllerRole::TrackedControllerRole_Invalid;
		break;
	case OpenVRController::ROLE::ROLE_LEFTHAND:
		openvrRoleId = vr::ETrackedControllerRole::TrackedControllerRole_LeftHand;
		break;
	case OpenVRController::ROLE::ROLE_RIGHTHAND:
		openvrRoleId = vr::ETrackedControllerRole::TrackedControllerRole_RightHand;
		break;
	}

	return openvrRoleId;
}

#endif
