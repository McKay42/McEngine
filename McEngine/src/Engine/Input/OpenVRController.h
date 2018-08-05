//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		openvr controller wrapper
//
// $NoKeywords: $vrc
//===============================================================================//

#ifndef OPENVRCONTROLLER_H
#define OPENVRCONTROLLER_H

#include "InputDevice.h"

#ifdef MCENGINE_FEATURE_OPENVR

#include <openvr_mingw.hpp>

#endif

class OpenVRController : public InputDevice
{
public:
	enum class BUTTON
	{
		BUTTON_SYSTEM,
		BUTTON_APPLICATIONMENU,
		BUTTON_GRIP,
		BUTTON_DPAD_LEFT,
		BUTTON_DPAD_UP,
		BUTTON_DPAD_RIGHT,
		BUTTON_DPAD_DOWN,
		BUTTON_A,
		BUTTON_PROXIMITYSENSOR,
		BUTTON_AXIS0,
		BUTTON_AXIS1,
		BUTTON_AXIS2,
		BUTTON_AXIS3,
		BUTTON_AXIS4,

		BUTTON_STEAMVR_TOUCHPAD = BUTTON_AXIS0,
		BUTTON_STEAMVR_TRIGGER = BUTTON_AXIS1,
		BUTTON_DASHBOARD_BACK = BUTTON_GRIP
	};

	enum class ROLE
	{
		ROLE_INVALID,
		ROLE_LEFTHAND,
		ROLE_RIGHTHAND
	};

	static bool STEAMVR_BUG_WORKAROUND_FLIPFLOP; // only allow 1 controller to send 1 TriggerHapticPulse() within 1 frame, buffer all other calls

public:

#if !defined(MCENGINE_FEATURE_OPENVR)

	OpenVRController();

#endif

#ifdef MCENGINE_FEATURE_OPENVR

	OpenVRController(vr::IVRSystem *hmd, OpenVRController::ROLE role);

	void update(uint64_t buttonPressed, uint64_t buttonTouched, vr::VRControllerAxis_t axes[vr::k_unControllerStateAxisCount]);
	void updateMatrixPose(Matrix4 &deviceToAbsoluteTracking);

	void setHmd(vr::IVRSystem *hmd) {m_hmd = hmd;}

#endif

	void updateDebug(float triggerValue);
	void updateMatrixPoseDebug(Vector3 pos, Vector3 forward, Vector3 up, Vector3 right);

	void triggerHapticPulse(unsigned short durationMicroSec = 500, OpenVRController::BUTTON button = OpenVRController::BUTTON::BUTTON_STEAMVR_TOUCHPAD);

	bool isButtonPressed(OpenVRController::BUTTON button);

	inline OpenVRController::ROLE getRole() const {return m_role;}

	inline Matrix4 getMatrixPose() const {return m_matrix;}
	inline Vector3 getPosition() const {return m_vPos;}
	inline Vector3 getDirection() const {return m_vDir;}
	inline Vector3 getUp() const {return m_vUp;}
	inline Vector3 getRight() const {return m_vRight;}

	float getTrigger();
	Vector2 getTouchpad();

#ifdef MCENGINE_FEATURE_OPENVR

	static vr::EVRButtonId buttonIdToOpenVR(OpenVRController::BUTTON buttonId);
	static vr::ETrackedControllerRole roleIdToOpenVR(OpenVRController::ROLE roleId);

#endif

private:
	ROLE m_role;
	Matrix4 m_matrix;

	Vector3 m_vPos;
	Vector3 m_vDir;
	Vector3 m_vUp;
	Vector3 m_vRight;

	float m_fLastTriggerHapticPulseTime;

#ifdef MCENGINE_FEATURE_OPENVR

	vr::IVRSystem *m_hmd;

	uint64_t m_ulButtonPressed;
	uint64_t m_ulButtonTouched;
	vr::VRControllerAxis_t m_rAxis[vr::k_unControllerStateAxisCount];

	struct TRIGGER_HAPTIC_PULSE_EVENT
	{
		unsigned short durationMicroSec;
		OpenVRController::BUTTON  button;
	};
	std::vector<TRIGGER_HAPTIC_PULSE_EVENT> m_triggerHapticPulseBuffer;

#endif
};

#endif
