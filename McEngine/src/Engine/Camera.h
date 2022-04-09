//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		3d quaternion camera system
//
// $NoKeywords: $cam
//===============================================================================//

#ifndef CAMERA_H
#define CAMERA_H

#include "cbase.h"
#include "Quaternion.h"

class Camera
{
public:
	static Matrix4 buildMatrixOrtho2D(float left, float right, float bottom, float top, float zn, float zf); // DEPRECATED (OpenGL specific)
	static Matrix4 buildMatrixOrtho2DGLLH(float left, float right, float bottom, float top, float zn, float zf); // OpenGL
	static Matrix4 buildMatrixOrtho2DDXLH(float left, float right, float bottom, float top, float zn, float zf); // DirectX
	static Matrix4 buildMatrixLookAt(Vector3 eye, Vector3 target, Vector3 up); // DEPRECATED
	static Matrix4 buildMatrixLookAtLH(Vector3 eye, Vector3 target, Vector3 up);
	static Matrix4 buildMatrixPerspectiveFov(float fovRad, float aspect, float zn, float zf); // DEPRECATED (OpenGL specific)
	static Matrix4 buildMatrixPerspectiveFovVertical(float fovRad, float aspectRatioWidthToHeight, float zn, float zf); // DEPRECATED
	static Matrix4 buildMatrixPerspectiveFovVerticalDXLH(float fovRad, float aspectRatioWidthToHeight, float zn, float zf);
	static Matrix4 buildMatrixPerspectiveFovHorizontal(float fovRad, float aspectRatioHeightToWidth, float zn, float zf); // DEPRECATED
	static Matrix4 buildMatrixPerspectiveFovHorizontalDXLH(float fovRad, float aspectRatioHeightToWidth, float zn, float zf);

public:
	enum CAMERA_TYPE
	{
		CAMERA_TYPE_FIRST_PERSON,
		CAMERA_TYPE_ORBIT
	};

	Camera(Vector3 pos = Vector3(0,0,0), Vector3 viewDir = Vector3(0,0,1), float fovDeg = 90.0f, CAMERA_TYPE camType = CAMERA_TYPE_FIRST_PERSON);

	void rotateX(float pitchDeg);
	void rotateY(float yawDeg);

	void lookAt(Vector3 target);

	// set
	void setType(CAMERA_TYPE camType);
	void setPos(Vector3 pos);
	void setFov(float fovDeg) {m_fFov = deg2rad(fovDeg);}
	void setFovRad(float fovRad) {m_fFov = fovRad;}
	void setOrbitDistance(float orbitDistance);
	void setOrbitYAxis(bool orbitYAxis) {m_bOrbitYAxis = orbitYAxis;}

	void setRotation(float yawDeg, float pitchDeg, float rollDeg);
	void setYaw(float yawDeg);
	void setPitch(float pitchDeg);
	void setRoll(float rollDeg);
	void setWorldOrientation(Quaternion worldRotation) {m_worldRotation = worldRotation; updateVectors();}

	// get
	inline CAMERA_TYPE getType() const {return m_camType;}
	inline Vector3 getPos() const {return m_vPos;}
	Vector3 getNextPosition(Vector3 velocity) const;

	inline float getFov() const {return rad2deg(m_fFov);}
	inline float getFovRad() const {return m_fFov;}
	inline float getOrbitDistance() const {return m_fOrbitDistance;}

	inline Vector3 getWorldXAxis() const {return m_worldRotation*m_vXAxis;}
	inline Vector3 getWorldYAxis() const {return m_worldRotation*m_vYAxis;}
	inline Vector3 getWorldZAxis() const {return m_worldRotation*m_vZAxis;}

	inline Vector3 getViewDirection() const {return m_vViewDir;}
	inline Vector3 getViewUp() const {return m_vViewUp;}
	inline Vector3 getViewRight() const {return m_vViewRight;}

	inline float getPitch() const {return m_fPitch;}
	inline float getYaw() const {return m_fYaw;}
	inline float getRoll() const {return m_fRoll;}

	inline Quaternion getRotation() const {return m_rotation;}

	Vector3 getProjectedVector(Vector3 point, float screenWidth, float screenHeight, float zn = 0.1f, float zf = 1.0f) const;
	Vector3 getUnProjectedVector(Vector2 point, float screenWidth, float screenHeight, float zn = 0.1f, float zf = 1.0f) const;

	bool isPointVisibleFrustum(Vector3 point) const; // within our viewing frustum
	bool isPointVisiblePlane(Vector3 point) const; // just in front of the camera plane

private:
	struct CAM_PLANE
	{
		float a,b,c,d;
	};

	static float planeDotCoord(CAM_PLANE plane, Vector3 point);
	static float planeDotCoord(Vector3 planeNormal, Vector3 planePoint, Vector3 &pv);

	void updateVectors();
	void updateViewFrustum();

	void lookAt(Vector3 eye, Vector3 target);

	// vars
	CAMERA_TYPE m_camType;
	Vector3 m_vPos;
	Vector3 m_vOrbitTarget;
	float m_fFov;
	float m_fOrbitDistance;
	bool m_bOrbitYAxis;

	// base axes
	Vector3 m_vWorldXAxis;
	Vector3 m_vWorldYAxis;
	Vector3 m_vWorldZAxis;

	// derived axes
	Vector3 m_vXAxis;
	Vector3 m_vYAxis;
	Vector3 m_vZAxis;

	// rotation
	Quaternion m_rotation;
	Quaternion m_worldRotation;
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	// relative coordinate system
	Vector3 m_vViewDir;
	Vector3 m_vViewRight;
	Vector3 m_vViewUp;

	// custom
	CAM_PLANE m_viewFrustum[4];
};

#endif
