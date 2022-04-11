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

	// This method just copies the data form another Camera, it does not change the Camera type
	void Camera::copyData(const Camera& other)
	{
		Type = other.Type;
		TypeStr = other.TypeStr;

		m_Position = other.m_Position;
		m_Target = other.m_Target;
		m_Up = other.m_Up;
		m_Right = other.m_Right;
		m_Front = other.m_Front;

		m_Yaw = other.m_Yaw;
		m_Pitch = other.m_Pitch;
		m_Roll = other.m_Roll;
		m_Fov = other.m_Fov;

		m_MovementSpeed = other.m_MovementSpeed;
		m_RollSpeed = other.m_RollSpeed;
		m_MouseSensitivity = other.m_MouseSensitivity;

		m_FrustumNear = other.m_FrustumNear;
		m_FrustumFar = other.m_FrustumFar;
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
