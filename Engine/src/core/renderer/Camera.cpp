// Camera.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Camera.h"

namespace Phoenix {
	Camera::Camera() :
		Type(CameraType::NONE),
		TypeStr("None")
	{
	}
	void Camera::setAngles(float yaw, float pitch, float roll)
	{
		m_Yaw = yaw;
		m_Pitch = pitch;
		m_Roll = roll;
	}

	void Camera::setFrustum(float frustum_near, float frustum_far)
	{
		m_FrustumNear = frustum_near;
		m_FrustumFar = frustum_far;
	}
}
