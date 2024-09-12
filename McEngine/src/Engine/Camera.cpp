//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		3d quaternion camera system
//
// $NoKeywords: $cam
//===============================================================================//

#include "Camera.h"

#include "Engine.h"
#include "ConVar.h"

ConVar cl_pitchup("cl_pitchup", 89.0f, FCVAR_CHEAT);
ConVar cl_pitchdown("cl_pitchdown", 89.0f, FCVAR_CHEAT);

Matrix4 Camera::buildMatrixOrtho2D(float left, float right, float bottom, float top, float zn, float zf)
{
	const float invX = 1.0f / (right - left);
	const float invY = 1.0f / (top - bottom);
	const float invZ = 1.0f / (zf - zn);

	return Matrix4((2.0f * invX),	0, 				0,				(-(right + left) * invX),
				   0, 				(2.0 * invY),	0,				(-(top + bottom) * invY),
				   0,				0,				(-2.0f * invZ),	(-(zf + zn) * invZ),
				   0,				0,				0,			   	1).transpose();
}

Matrix4 Camera::buildMatrixOrtho2DGLLH(float left, float right, float bottom, float top, float zn, float zf)
{
	const float invX = 1.0f / (right - left);
	const float invY = 1.0f / (top - bottom);
	const float invZ = 1.0f / (zf - zn);

	return Matrix4((2.0f * invX),	0, 				0,				(-(right + left) * invX),
				   0, 				(2.0 * invY),	0,				(-(top + bottom) * invY),
				   0,				0,				(-2.0f * invZ),	(-(zf + zn) * invZ),
				   0,				0,				0,			   	1).transpose();
}

Matrix4 Camera::buildMatrixOrtho2DDXLH(float left, float right, float bottom, float top, float zn, float zf)
{
	return Matrix4(2.0f/(right - left),		0, 						0,					(left + right)/(left - right),
				   0, 						2.0f/(top - bottom),	0,					(top + bottom)/(bottom - top),
				   0,						0,						1.0f/(zf - zn),		zn/(zn - zf),
				   0,						0,						0,			   		1).transpose();
}

Matrix4 Camera::buildMatrixLookAt(Vector3 eye, Vector3 target, Vector3 up)
{
	const Vector3 zAxis = (eye - target).normalize();
	const Vector3 xAxis = up.cross(zAxis).normalize();
	const Vector3 yAxis = zAxis.cross(xAxis);

	return Matrix4(xAxis.x, xAxis.y, xAxis.z, -xAxis.dot(eye),
				   yAxis.x, yAxis.y, yAxis.z, -yAxis.dot(eye),
				   zAxis.x, zAxis.y, zAxis.z, -zAxis.dot(eye),
				   0,		0,		 0,		   1).transpose();
}

Matrix4 Camera::buildMatrixLookAtLH(Vector3 eye, Vector3 target, Vector3 up)
{
	const Vector3 zAxis = (target - eye).normalize();
	const Vector3 xAxis = up.cross(zAxis).normalize();
	const Vector3 yAxis = zAxis.cross(xAxis);

	return Matrix4(xAxis.x, xAxis.y, xAxis.z, -xAxis.dot(eye),
				   yAxis.x, yAxis.y, yAxis.z, -yAxis.dot(eye),
				   zAxis.x, zAxis.y, zAxis.z, -zAxis.dot(eye),
				   0,		0,		 0,		   1).transpose();
}

Matrix4 Camera::buildMatrixPerspectiveFov(float fovRad, float aspect, float zn, float zf)
{
	const float f = 1.0f / std::tan(fovRad / 2.0f);

	const float q = (zf + zn) / (zn - zf);
	const float qn = (2 * zf * zn) / (zn - zf);

	return Matrix4(f / aspect,	0,		0, 		0,
				   0, 			f, 		0,		0,
				   0, 			0,		q,		qn,
				   0,			0,		-1,		0).transpose();
}

Matrix4 Camera::buildMatrixPerspectiveFovVertical(float fovRad, float aspectRatioWidthToHeight, float zn, float zf)
{
	const float f = 1.0f / std::tan(fovRad / 2.0f);

	const float q = (zf + zn) / (zn - zf);
	const float qn = (2 * zf * zn) / (zn - zf);

	return Matrix4(f / aspectRatioWidthToHeight,	0,		0, 		0,
				   0, 								f, 		0,		0,
				   0, 								0,		q,		qn,
				   0,								0,		-1,		0).transpose();
}

Matrix4 Camera::buildMatrixPerspectiveFovVerticalDXLH(float fovRad, float aspectRatioWidthToHeight, float zn, float zf)
{
	const float f = 1.0f / std::tan(fovRad / 2.0f);

	return Matrix4(f / aspectRatioWidthToHeight,	0,		0, 				0,
				   0, 								f, 		0,				0,
				   0, 								0,		zf/(zf - zn),	-zn*zf/(zf - zn),
				   0,								0,		1,				0).transpose();
}

Matrix4 Camera::buildMatrixPerspectiveFovHorizontal(float fovRad, float aspectRatioHeightToWidth, float zn, float zf)
{
	const float f = 1.0f / std::tan(fovRad / 2.0f);

	const float q = (zf + zn) / (zn - zf);
	const float qn = (2 * zf * zn) / (zn - zf);

	return Matrix4(f,	0,								0, 		0,
				   0, 	f / aspectRatioHeightToWidth,	0,		0,
				   0, 	0,								q,		qn,
				   0,	0,								-1,		0).transpose();
}

Matrix4 Camera::buildMatrixPerspectiveFovHorizontalDXLH(float fovRad, float aspectRatioHeightToWidth, float zn, float zf)
{
	const float f = 1.0f / std::tan(fovRad / 2.0f);

	return Matrix4(f,	0,								0, 				0,
				   0, 	f / aspectRatioHeightToWidth, 	0,				0,
				   0, 	0,								zf/(zf - zn),	-zn*zf/(zf - zn),
				   0,	0,								1,				0).transpose();
}

/*
static Quaternion MatrixToQuaternion(const Matrix4 &in)
{
	const float trace = in[0] + in[5] + in[10];

	Quaternion final;
	if (trace > 0.0f)
	{
		const float s = 0.5f / std::sqrt(trace + 1.0f);

		final.w = 0.25f / s;
		final.x = (in[9]- in[6]) * s;
		final.y = (in[2] - in[8]) * s;
		final.z = (in[4] - in[1]) * s;
	}
	else
	{
		if (in[0] > in[5] && in[0] > in[10])
		{
			const float s = 2.0f * std::sqrt(1.0f + in[0] - in[5] - in[10]);

			final.w = (in[9] - in[6]) / s;
			final.x = 0.25f * s;
			final.y = (in[1] + in[4]) / s;
			final.z = (in[2] + in[8]) / s;
		}
		else if (in[5] > in[10])
		{
			const float s = 2.0f * std::sqrt(1.0f + in[5] - in[0] - in[10]);

			final.w = (in[2] - in[8]) / s;
			final.x = (in[1] + in[4]) / s;
			final.y = 0.25f * s;
			final.z = (in[6] + in[9]) / s;
		}
		else
		{
			const float s = 2.0f * std::sqrt(1.0f + in[10] - in[0] - in[5]);

			final.w = (in[4] - in[1]) / s;
			final.x = (in[2] + in[8]) / s;
			final.y = (in[6] + in[9]) / s;
			final.z = 0.25f * s;
		}
	}

	return final;

	//float qw = std::sqrt(1 + in[0] + in[5] + in[10]) / 2.0f;
	//float qx = (in[9] - in[6]) / (4 *qw);
	//float qy = (in[2] - in[8]) / (4 *qw);
	//float qz = (in[4] - in[1]) / (4 *qw);

	//return Quaternion(qx, qy, qz, qw);
}
*/

static Vector3 Vector3TransformCoord(const Vector3 &in, const Matrix4 &mat)
{
	//return mat * in; // wtf?

	Vector3 out;

	const float norm = mat[3]*in.x + mat[7]*in.y + mat[11]*in.z + mat[15];

	if (norm)
	{
		out.x = (mat[0]*in.x + mat[4]*in.y + mat[8]*in.z + mat[12]) / norm;
		out.y = (mat[1]*in.x + mat[5]*in.y + mat[9]*in.z + mat[13]) / norm;
		out.z = (mat[2]*in.x + mat[6]*in.y + mat[10]*in.z + mat[14]) / norm;
	}

	return out;
}



//*************************//
//	Camera implementation  //
//*************************//

Camera::Camera(Vector3 pos, Vector3 viewDir, float fovDeg, CAMERA_TYPE camType)
{
	m_vPos = pos;
	m_vViewDir = viewDir;
	m_fFov = deg2rad(fovDeg);
	m_camType = camType;

	m_fOrbitDistance = 5.0f;
	m_bOrbitYAxis = true;

	m_fPitch = 0;
	m_fYaw = 0;
	m_fRoll = 0;

	// base axes
	m_vWorldXAxis = Vector3(1, 0, 0);
	m_vWorldYAxis = Vector3(0, 1, 0);
	m_vWorldZAxis = Vector3(0, 0, 1);

	// derived axes
	m_vXAxis = m_vWorldXAxis;
	m_vYAxis = m_vWorldYAxis;
	m_vZAxis = m_vWorldZAxis;

	m_vViewRight = m_vWorldXAxis;
	m_vViewUp = m_vWorldYAxis;

	lookAt(pos + viewDir);
}

void Camera::updateVectors()
{
	// update rotation
	if (m_camType == CAMERA_TYPE_FIRST_PERSON)
		m_rotation.fromEuler(m_fRoll, m_fYaw, -m_fPitch);
	else if (m_camType == CAMERA_TYPE_ORBIT)
	{
		m_rotation.identity();

		if (m_bOrbitYAxis)
		{
			// yaw
			Quaternion tempQuat;
			tempQuat.fromAxis(m_vYAxis, m_fYaw);

			m_rotation = tempQuat * m_rotation;

			// pitch
			tempQuat.fromAxis(m_vXAxis, -m_fPitch);
			m_rotation = m_rotation * tempQuat;

			m_rotation.normalize();
		}
	}

	// calculate new coordinate system
	m_vViewDir = (m_worldRotation*m_rotation) * m_vZAxis;
	m_vViewRight = (m_worldRotation*m_rotation) * m_vXAxis;
	m_vViewUp = (m_worldRotation*m_rotation) * m_vYAxis;

	// update pos if we are orbiting (with the new coordinate system from above)
	if (m_camType == CAMERA_TYPE_ORBIT)
		setPos(m_vOrbitTarget);

	updateViewFrustum();
}

void Camera::updateViewFrustum()
{
	// TODO: this function is broken due to matrix changes, refactor

	const Matrix4 viewMatrix = m_rotation.getMatrix();

	const Matrix4 projectionMatrix = buildMatrixPerspectiveFov(m_fFov, (float)engine->getScreenWidth() / (float)engine->getScreenHeight(), 0.0f, 1.0f);

	const Matrix4 viewProj = (viewMatrix * projectionMatrix);

	// left plane
    m_viewFrustum[0].a = viewProj[3] + viewProj[0];
    m_viewFrustum[0].b = viewProj[7] + viewProj[4];
    m_viewFrustum[0].c = viewProj[11] + viewProj[8];
    m_viewFrustum[0].d = viewProj[15] + viewProj[12];

    // right plane
    m_viewFrustum[1].a = viewProj[3] - viewProj[0];
    m_viewFrustum[1].b = viewProj[7] - viewProj[4];
    m_viewFrustum[1].c = viewProj[11] - viewProj[8];
    m_viewFrustum[1].d = viewProj[15] - viewProj[12];

    // top plane
    m_viewFrustum[2].a = viewProj[3] - viewProj[1];
    m_viewFrustum[2].b = viewProj[7] - viewProj[5];
    m_viewFrustum[2].c = viewProj[11] - viewProj[9];
    m_viewFrustum[2].d = viewProj[15] - viewProj[13];

    // bottom plane
    m_viewFrustum[3].a = viewProj[3] + viewProj[1];
    m_viewFrustum[3].b = viewProj[7] + viewProj[5];
    m_viewFrustum[3].c = viewProj[11] + viewProj[9];
    m_viewFrustum[3].d = viewProj[15] + viewProj[13];

	// normalize planes
    for (int i=0; i<4; i++)
    {
    	const float norm = std::sqrt(m_viewFrustum[i].a * m_viewFrustum[i].a + m_viewFrustum[i].b * m_viewFrustum[i].b + m_viewFrustum[i].c * m_viewFrustum[i].c);
    	if (norm)
    	{
    		m_viewFrustum[i].a = m_viewFrustum[i].a / norm;
    		m_viewFrustum[i].b = m_viewFrustum[i].b / norm;
    		m_viewFrustum[i].c = m_viewFrustum[i].c / norm;
    		m_viewFrustum[i].d = m_viewFrustum[i].d / norm;
    	}
    	else
    	{
    		m_viewFrustum[i].a = 0.0f;
    		m_viewFrustum[i].b = 0.0f;
    		m_viewFrustum[i].c = 0.0f;
    		m_viewFrustum[i].d = 0.0f;
    	}
    }
}

void Camera::rotateX(float pitchDeg)
{
	m_fPitch += pitchDeg;

	if (m_fPitch > cl_pitchup.getFloat())
		m_fPitch = cl_pitchup.getFloat();
	else if (m_fPitch < -cl_pitchdown.getFloat())
		m_fPitch = -cl_pitchdown.getFloat();

	updateVectors();
}

void Camera::rotateY(float yawDeg)
{
	m_fYaw += yawDeg;

	if (m_fYaw > 360.0f)
		m_fYaw = m_fYaw - 360.0f;
	else if (m_fYaw < 0.0f)
		m_fYaw = 360.0f + m_fYaw;

	updateVectors();
}

void Camera::lookAt(Vector3 target)
{
	lookAt(m_vPos, target);
}

void Camera::lookAt(Vector3 eye, Vector3 target)
{
	if ((eye - target).length() < 0.001f) return;

	m_vPos = eye;

	// https://stackoverflow.com/questions/1996957/conversion-euler-to-matrix-and-matrix-to-euler
	// https://gamedev.stackexchange.com/questions/50963/how-to-extract-euler-angles-from-transformation-matrix

	Matrix4 lookAtMatrix = buildMatrixLookAt(eye, target, m_vYAxis);

	float yaw = std::atan2(-lookAtMatrix[8], lookAtMatrix[0]);
	float pitch = std::asin(-lookAtMatrix[6]);
	///float roll = atan2(lookAtMatrix[4], lookAtMatrix[5]);

	m_fYaw = 180.0f + rad2deg(yaw);
	m_fPitch = rad2deg(pitch);

	updateVectors();
}

void Camera::setType(CAMERA_TYPE camType)
{
	if (camType == m_camType) return;

	m_camType = camType;

	if (m_camType == CAMERA_TYPE_ORBIT)
		setPos(m_vOrbitTarget);
	else
		m_vPos = m_vOrbitTarget;
}

void Camera::setPos(Vector3 pos)
{
	m_vOrbitTarget = pos;

	if (m_camType == CAMERA_TYPE_ORBIT)
		m_vPos = m_vOrbitTarget + m_vViewDir * -m_fOrbitDistance;
	else if (m_camType == CAMERA_TYPE_FIRST_PERSON)
		m_vPos = pos;
}

void Camera::setOrbitDistance(float orbitDistance)
{
	 m_fOrbitDistance = orbitDistance;
	 if (m_fOrbitDistance < 0)
		 m_fOrbitDistance = 0;
}

void Camera::setRotation(float yawDeg, float pitchDeg, float rollDeg)
{
	m_fYaw = yawDeg;
	m_fPitch = pitchDeg;
	m_fRoll = rollDeg;
	updateVectors();
}

void Camera::setYaw(float yawDeg)
{
	m_fYaw = yawDeg;
	updateVectors();
}

void Camera::setPitch(float pitchDeg)
{
	m_fPitch = pitchDeg;
	updateVectors();
}

void Camera::setRoll(float rollDeg)
{
	m_fRoll = rollDeg;
	updateVectors();
}

Vector3 Camera::getNextPosition(Vector3 velocity) const
{
	return m_vPos + ((m_worldRotation * m_rotation) * velocity);
}

Vector3 Camera::getProjectedVector(Vector3 point, float screenWidth, float screenHeight, float zn, float zf) const
{
	// TODO: fix this shit

	// build matrices
	const Matrix4 worldMatrix = Matrix4().translate(-m_vPos);
	const Matrix4 viewMatrix = m_rotation.getMatrix();
	const Matrix4 projectionMatrix = buildMatrixPerspectiveFov(m_fFov, screenWidth / screenHeight, zn, zf);

	// complete matrix
	Matrix4 worldViewProj = (worldMatrix * viewMatrix * projectionMatrix);

	// transform 3d point to 2d
	point = Vector3TransformCoord(point, worldViewProj);

	// convert projected screen coordinates to real screen coordinates
	point.x = screenWidth * ((point.x + 1.0f) / 2.0f);
	point.y = screenHeight * ((point.y + 1.0f) / 2.0f);
	point.z = zn + point.z * (zf - zn);

	return point;
}

Vector3 Camera::getUnProjectedVector(Vector2 point, float screenWidth, float screenHeight, float zn, float zf) const
{
	Matrix4 projectionMatrix = buildMatrixPerspectiveFov(m_fFov, screenWidth / screenHeight, zn, zf);

	// transform pick position from screen space into 3d space
	Vector4 v;
	v.x = ( ( 2.0f * point.x ) / screenWidth ) - 1.0f;
	v.y = ( ( 2.0f * point.y ) / screenHeight ) - 1.0f;
	v.z = (0.0f - zn) / (zf - zn); // this is very important
	v.w = 1.0f;

	const Matrix4 inverseProjectionMatrix = projectionMatrix.invert();
	const Matrix4 inverseViewMatrix = m_rotation.getMatrix().invert();

	v = inverseViewMatrix * inverseProjectionMatrix * v;

	return m_vPos - Vector3(v.x, v.y, v.z);
}

bool Camera::isPointVisibleFrustum(Vector3 point) const
{
	float epsilon = 0.01f;

	// left
	float d11 = planeDotCoord(m_viewFrustum[0], point);

	// right
	float d21 = planeDotCoord(m_viewFrustum[1], point);

	// top
	float d31 = planeDotCoord(m_viewFrustum[2], point);

	// bottom
	float d41 = planeDotCoord(m_viewFrustum[3], point);

	if ((d11 < epsilon) || (d21 < epsilon) || (d31 < epsilon) || (d41 < epsilon))
		return false;

	return true;
}

bool Camera::isPointVisiblePlane(Vector3 point) const
{
	float epsilon = 0.0f;

	if (!(planeDotCoord(m_vViewDir, m_vPos, point) < epsilon))
		return true;

	return false;
}

float Camera::planeDotCoord(CAM_PLANE plane, Vector3 point)
{
	return ((plane.a * point.x) + (plane.b * point.y) + (plane.c * point.z) + plane.d);
}

float Camera::planeDotCoord(Vector3 planeNormal, Vector3 planePoint, Vector3 &pv)
{
	return planeNormal.dot(pv - planePoint);
}
