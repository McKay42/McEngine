//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		a simple quaternion
//
// $NoKeywords: $quat
//===============================================================================//

#include "Quaternion.h"

void Quaternion::set(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

void Quaternion::normalize()
{
	float magnitudeSquared = w * w + x * x + y * y + z * z;
	if (std::abs(magnitudeSquared) > EPSILON && std::abs(magnitudeSquared - 1.0f) > EPSILON)
	{
		float magnitude = std::sqrt(magnitudeSquared);
		w /= magnitude;
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}
}

void Quaternion::fromAxis(const Vector3 &axis, float angleDeg)
{
	angleDeg = deg2rad(angleDeg);

	float sinAngle;
	angleDeg *= 0.5f;

	Vector3 axisCopy(axis);
	axisCopy.normalize();

	sinAngle = std::sin(angleDeg);

	x = (axisCopy.x * sinAngle);
	y = (axisCopy.y * sinAngle);
	z = (axisCopy.z * sinAngle);
	w = std::cos(angleDeg);
}

void Quaternion::fromEuler(float yawDeg, float pitchDeg, float rollDeg)
{
	float y = yawDeg * PIOVER180 / 2.0f;
	float p = pitchDeg * PIOVER180 / 2.0f;
	float r = rollDeg * PIOVER180 / 2.0f;

	float sinYaw = std::sin(y);
	float sinPitch = std::sin(p);
	float sinRoll = std::sin(r);

	float cosYaw = std::cos(y);
	float cosPitch = std::cos(p);
	float cosRoll = std::cos(r);

	this->x = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
	this->y = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
	this->z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
	this->w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;

	normalize();
}

Matrix4 Quaternion::getMatrix() const
{
	return Matrix4(1.0f - 2.0f*y*y - 2.0f*z*z, 2.0f*x*y - 2.0f*z*w, 2.0f*x*z + 2.0f*y*w, 0.0f,
						  2.0f*x*y + 2.0f*z*w, 1.0f - 2.0f*x*x - 2.0f*z*z, 2.0f*y*z - 2.0f*x*w, 0.0f,
						  2.0f*x*z - 2.0f*y*w, 2.0f*y*z + 2.0f*x*w, 1.0f - 2.0f*x*x - 2.0f*y*y, 0.0f,
						  0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix3 Quaternion::getMatrix3() const
{
	return Matrix3(1.0f - 2.0f*y*y - 2.0f*z*z, 2.0f*x*y - 2.0f*z*w, 2.0f*x*z + 2.0f*y*w,
				   2.0f*x*y + 2.0f*z*w, 1.0f - 2.0f*x*x - 2.0f*z*z, 2.0f*y*z - 2.0f*x*w,
				   2.0f*x*z - 2.0f*y*w, 2.0f*y*z + 2.0f*x*w, 1.0f - 2.0f*x*x - 2.0f*y*y);
}

Quaternion Quaternion::operator * (const Quaternion &quat) const
{
	return Quaternion(w * quat.x + x * quat.w + y * quat.z - z * quat.y,
	                  w * quat.y + y * quat.w + z * quat.x - x * quat.z,
	                  w * quat.z + z * quat.w + x * quat.y - y * quat.x,
	                  w * quat.w - x * quat.x - y * quat.y - z * quat.z);
}

Vector3 Quaternion::operator * (const Vector3 &vec) const
{
	Vector3 vecCopy(vec);

	Quaternion vecQuaternion, resQuaternion;
	vecQuaternion.x = vecCopy.x;
	vecQuaternion.y = vecCopy.y;
	vecQuaternion.z = vecCopy.z;
	vecQuaternion.w = 0.0f;

	resQuaternion = vecQuaternion * getConjugate();
	resQuaternion = *this * resQuaternion;

	return (Vector3(resQuaternion.x, resQuaternion.y, resQuaternion.z));
}
