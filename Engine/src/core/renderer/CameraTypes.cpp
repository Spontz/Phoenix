// CameraTypes.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Camera.h"
#include "core/renderer/CameraTypes.h"

namespace Phoenix {

	/////////
	// Projection Free Camera: Like an "FPS" camera: The target is not locked, the target is changed with the Euler Angles
	CameraProjectionFPS::CameraProjectionFPS(glm::vec3 const& position)
	{
		Type = CameraType::PROJ_FPS;
		TypeStr = "Projection FPS";

		m_Position = position;
	}

	const glm::mat4 CameraProjectionFPS::getProjection()
	{
		return glm::perspective(glm::radians(m_Fov), GLDRV->GetFramebufferViewport().GetAspectRatio(), m_FrustumNear, m_FrustumFar);
	}

	const glm::mat4 CameraProjectionFPS::getView()
	{
		updateCameraVectors(); // Calculate the Front position considering the Euler angles
		return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
	}

	void CameraProjectionFPS::processKeyboard(CameraMovement direction, float deltaTime)
	{
		const float velocity = m_MovementSpeed * deltaTime;
		const float velocity_roll = m_RollSpeed * deltaTime;


		switch (direction) {
		case CameraMovement::FORWARD:
			m_Position += m_Front * velocity;
			break;
		case CameraMovement::BACKWARD:
			m_Position -= m_Front * velocity;
			break;
		case CameraMovement::LEFT:
			m_Position -= m_Right * velocity;
			break;
		case CameraMovement::RIGHT:
			m_Position += m_Right * velocity;
			break;
		case CameraMovement::ROLL_LEFT:
			m_Roll += velocity_roll;
			break;
		case CameraMovement::ROLL_RIGHT:
			m_Roll -= velocity_roll;
			break;
		}
		//updateCameraVectors();
	}

	void CameraProjectionFPS::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
	{
		xoffset *= m_MouseSensitivity;
		yoffset *= m_MouseSensitivity;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		//updateCameraVectors();
	}

	void CameraProjectionFPS::processMouseScroll(float yoffset)
	{
		m_Fov -= yoffset;
		if (m_Fov < 1.0f)
			m_Fov = 1.0f;
		if (m_Fov > 179.0f)
			m_Fov = 179.0f;
	}

	void CameraProjectionFPS::multiplyMovementSpeed(float speed)
	{
		m_MovementSpeed *= speed;
	}

	void CameraProjectionFPS::divideMovementSpeed(float speed)
	{
		m_MovementSpeed /= speed;
	}

	void CameraProjectionFPS::capturePos()
	{
		std::ofstream camFile;
		//string message;
		char message[1024];
		camFile.open("camera_fps.cam", std::ios::out | std::ios::app);
		sprintf_s(message, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",
			m_Position.x, m_Position.y, m_Position.z,
			m_Up.x, m_Up.y, m_Up.z,
			m_Yaw, m_Pitch, m_Roll, m_Fov,
			m_FrustumNear, m_FrustumFar);
		camFile << message << "\n";
		camFile.close();
	}

	void CameraProjectionFPS::reset()
	{
		m_Position = DEFAULT_CAM_POSITION;
		m_Target = DEFAULT_CAM_TARGET;
		m_Up = DEFAULT_CAM_UP;
		m_Right = DEFAULT_CAM_RIGHT;

		// Euler angles
		m_Yaw = DEFAULT_CAM_YAW;
		m_Pitch = DEFAULT_CAM_PITCH;
		m_Roll = DEFAULT_CAM_ROLL;
		m_Fov = DEFAULT_CAM_FOV;

		// Camera behaviour
		m_MovementSpeed = DEFAULT_CAM_MOVEMENT_SPEED;
		m_RollSpeed = DEFAULT_CAM_ROLL_SPEED;
		m_MouseSensitivity = DEFAULT_CAM_SENSITIVITY;

		// Frustum
		m_FrustumNear = DEFAULT_CAM_NEAR;
		m_FrustumFar = DEFAULT_CAM_FAR;
	}

	void CameraProjectionFPS::setRollMatrix(glm::mat3& m, glm::vec3 f)
	{
		float rcos = glm::cos(glm::radians(m_Roll));
		float rsin = glm::sin(glm::radians(m_Roll));

		m[0][0] = rcos + (1 - rcos) * f.x * f.x;
		m[0][1] = (1 - rcos) * f.x * f.y + rsin * f.z;
		m[0][2] = (1 - rcos) * f.z * f.x - rsin * f.y;

		m[1][0] = (1 - rcos) * f.x * f.y - rsin * f.z;
		m[1][1] = rcos + (1 - rcos) * f.y * f.y;
		m[1][2] = (1 - rcos) * f.z * f.y + rsin * f.x;

		m[2][0] = (1 - rcos) * f.x * f.z + rsin * f.y;
		m[2][1] = (1 - rcos) * f.y * f.z - rsin * f.x;
		m[2][2] = rcos + (1 - rcos) * f.z * f.z;
	}

	void CameraProjectionFPS::updateCameraVectors()
	{

		// Calculate the new Front vector
		glm::vec3 front;
		front.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
		front.y = glm::sin(glm::radians(m_Pitch));
		front.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
		m_Front = glm::normalize(front);

		//	control the rotate view
		glm::mat3 rollMatrix;
		setRollMatrix(rollMatrix, front);
		// Also re-calculate the Right and Up vector
		m_Right = glm::normalize(glm::cross(m_Front, rollMatrix * m_WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_Up = glm::normalize(glm::cross(m_Right, m_Front));
	}


	/////////
	// Projection Target Camera: Similar to ProjectionFree camera, but target can be specified manually
	CameraProjectionTarget::CameraProjectionTarget(glm::vec3 const& position, glm::vec3 const& target)
	{
		Type = CameraType::PROJ_TARGET;
		TypeStr = "Projection target";

		m_Position = position;
		m_Target = target;
	}

	const glm::mat4 CameraProjectionTarget::getProjection()
	{
		return glm::perspective(glm::radians(m_Fov), GLDRV->GetFramebufferViewport().GetAspectRatio(), m_FrustumNear, m_FrustumFar);
	}

	const glm::mat4 CameraProjectionTarget::getView()
	{
		m_Front = glm::normalize(m_Target - m_Position);
		const auto m1 = glm::lookAt({ 0,0,0 }, m_Front, m_Up); // Up should be always normalized
		const auto mRot = glm::orientate4(glm::vec3(glm::radians(m_Pitch), glm::radians(m_Roll), glm::radians(m_Yaw)));
		const auto mTrans = glm::translate(-m_Position);
		return (m1 * mRot * mTrans);
	}

	void CameraProjectionTarget::processKeyboard(CameraMovement direction, float deltaTime)
	{
		const float velocity = m_MovementSpeed * deltaTime;
		const float velocity_roll = m_RollSpeed * deltaTime;


		switch (direction) {
		case CameraMovement::FORWARD:
			m_Position += glm::vec3(0,0,1) * velocity;
			break;
		case CameraMovement::BACKWARD:
			m_Position -= glm::vec3(0,0,1) * velocity;
			break;
		case CameraMovement::LEFT:
			m_Position -= glm::vec3(1, 0, 0) * velocity;
			break;
		case CameraMovement::RIGHT:
			m_Position += glm::vec3(1, 0, 0) * velocity;
			break;
		case CameraMovement::ROLL_LEFT:
			m_Roll += velocity_roll;
			break;
		case CameraMovement::ROLL_RIGHT:
			m_Roll -= velocity_roll;
			break;
		}
	}

	void CameraProjectionTarget::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
	{
		xoffset *= m_MouseSensitivity;
		yoffset *= m_MouseSensitivity;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}
	}

	void CameraProjectionTarget::processMouseScroll(float yoffset)
	{
		m_Fov -= yoffset;
		if (m_Fov < 1.0f)
			m_Fov = 1.0f;
		if (m_Fov > 179.0f)
			m_Fov = 179.0f;
	}

	void CameraProjectionTarget::multiplyMovementSpeed(float speed)
	{
		m_MovementSpeed *= speed;
	}

	void CameraProjectionTarget::divideMovementSpeed(float speed)
	{
		m_MovementSpeed /= speed;
	}

	void CameraProjectionTarget::capturePos()
	{
		std::ofstream camFile;
		//string message;
		char message[1024];
		camFile.open("camera_target.cam", std::ios::out | std::ios::app);
		sprintf_s(message, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",
			m_Position.x, m_Position.y, m_Position.z,
			m_Target.x, m_Target.y, m_Target.z,
			m_Up.x, m_Up.y, m_Up.z,
			m_Yaw, m_Pitch, m_Roll, m_Fov,
			m_FrustumNear, m_FrustumFar);
		camFile << message << "\n";
		camFile.close();
	}

	void CameraProjectionTarget::reset()
	{
		m_Position = DEFAULT_CAM_POSITION;
		m_Target = DEFAULT_CAM_TARGET;
		m_Up = DEFAULT_CAM_UP;
		m_Right = DEFAULT_CAM_RIGHT;

		// Euler angles
		m_Yaw = DEFAULT_CAM_YAW;
		m_Pitch = DEFAULT_CAM_PITCH;
		m_Roll = DEFAULT_CAM_ROLL;
		m_Fov = DEFAULT_CAM_FOV;

		// Camera behaviour
		m_MovementSpeed = DEFAULT_CAM_MOVEMENT_SPEED;
		m_RollSpeed = DEFAULT_CAM_ROLL_SPEED;
		m_MouseSensitivity = DEFAULT_CAM_SENSITIVITY;

		// Frustum
		m_FrustumNear = DEFAULT_CAM_NEAR;
		m_FrustumFar = DEFAULT_CAM_FAR;
	}


	/////////
	// Orthogonal Camera
	CameraOrthogonal::CameraOrthogonal()
	{
		Type = CameraType::ORTHOGONAL;
		TypeStr = "Orthogonal";
	}

	const glm::mat4 CameraOrthogonal::getProjection()
	{
		return glm::ortho(-1, 1, 1, -1, -1, 2);
	}
	const glm::mat4 CameraOrthogonal::getView()
	{
		return glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	}

	/////////
	// Raw Matrix Camera
	CameraRawMatrix::CameraRawMatrix(glm::mat4 const& matrix)
	{
		Type = CameraType::RAW_MATRIX;
		TypeStr = "Raw Matrix";
		m_Matrix = matrix;
	}
	const glm::mat4 CameraRawMatrix::getProjection()
	{
		return glm::perspective(glm::radians(m_Fov), GLDRV->GetFramebufferViewport().GetAspectRatio(), m_FrustumNear, m_FrustumFar);
	}
	const glm::mat4 CameraRawMatrix::getView()
	{
		return m_Matrix;
	}

}
