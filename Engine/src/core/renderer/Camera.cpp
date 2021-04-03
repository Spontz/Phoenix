// Camera.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Camera.h"

namespace Phoenix {

	Camera::Camera(glm::vec3 const& position, float yaw, float pitch, float roll)
	{
		camType = CameraType::FREE;
		Name = "Free Camera";
		
		MovementSpeed = DEFAULT_CAM_SPEED;
		RollSpeed = DEFAULT_CAM_ROLL_SPEED;
		MouseSensitivity = DEFAULT_CAM_SENSITIVITY;
		Fov = DEFAULT_CAM_VFOV;
		Near = DEFAULT_CAM_NEAR;
		Far = DEFAULT_CAM_FAR;

		Position = position;
		WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		Yaw = yaw;
		Pitch = pitch;
		Roll = roll;

		
		updateCameraVectors();
	}

	Camera::Camera(glm::vec3 const& position, glm::vec3 const& at, glm::vec3 const& up, float yaw, float pitch, float roll)
	{
		camType = CameraType::TARGET;
		Name = "Target + Free";
		
		MovementSpeed = DEFAULT_CAM_SPEED;
		RollSpeed = DEFAULT_CAM_ROLL_SPEED;
		MouseSensitivity = DEFAULT_CAM_SENSITIVITY;
		Fov = DEFAULT_CAM_VFOV;
		Near = DEFAULT_CAM_NEAR;
		Far = DEFAULT_CAM_FAR;

		Position = position;
		At = at;
		Up = up;
		Yaw = yaw;
		Pitch = pitch;
		Roll = roll;

		//updateCameraVectors();
	}

	Camera::Camera(glm::mat4 const& matrix)
	{
		Name = "Matrix Raw";
		Matrix = matrix;
	}

	void Camera::reset()
	{
		// TODO: Fix this
		//camType = CameraType::FREE;

		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		MovementSpeed = DEFAULT_CAM_SPEED;
		MouseSensitivity = DEFAULT_CAM_SENSITIVITY;
		Fov = DEFAULT_CAM_VFOV;
		Near = DEFAULT_CAM_NEAR;
		Far = DEFAULT_CAM_FAR;
		Yaw = DEFAULT_CAM_YAW;
		Pitch = DEFAULT_CAM_PITCH;
		Roll = DEFAULT_CAM_ROLL;

		Position = glm::vec3(0.0f, 0.0f, 3.0f);
		WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		updateCameraVectors();
	}

	glm::mat4 Camera::getViewMatrix() const // TODO: Implementar en el mismo getViewMatrix el obtener la matriz 'raw' (la que devielve con el getMatrix), y quitar el método getMatrix
	{
		switch (camType)
		{
		case CameraType::FREE:
			return glm::lookAt(Position, Position + Front, Up);
		case CameraType::TARGET:
			Front = normalize(At - Position);
			//const auto m1 = glm::lookAt({ 0,0,0 }, forward, m_vCamUp); // m_vCamUp should be normalized
			//
			const auto m1 = glm::orientation(Front, Up); // m_vCamUp should be normalized
			const auto mRot = glm::orientate4(glm::vec3(Pitch, Roll, Yaw));
			const auto mTrans = glm::translate(-Position);
			return (m1 * mRot * mTrans);
			
		case CameraType::RAW_MATRIX:
			return Matrix;
		default:
			throw std::runtime_error("Invalid camera type");
		}
		
	}

	glm::mat4 Camera::getProjectionMatrix() const
	{
		return glm::perspective(glm::radians(Fov), GLDRV->GetFramebufferViewport().GetAspectRatio(), Near, Far);
	}

	glm::mat4 Camera::getOrthoProjectionMatrix() const
	{
		return glm::ortho(-1, 1, 1, -1, -1, 2);
	}

	glm::mat4 Camera::getOrthoViewMatrix() const
	{
		return glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	}

	glm::mat4 Camera::getMatrix() const
	{
		return Matrix;
	}

	void Camera::processKeyboard(CameraMovement direction, float deltaTime)
	{
		const float velocity = MovementSpeed * deltaTime;
		const float velocity_roll = RollSpeed * deltaTime;


		switch (direction) {
		case CameraMovement::FORWARD:
			Position += Front * velocity;
			break;
		case CameraMovement::BACKWARD:
			Position -= Front * velocity;
			break;
		case CameraMovement::LEFT:
			Position -= Right * velocity;
			break;
		case CameraMovement::RIGHT:
			Position += Right * velocity;
			break;
		case CameraMovement::ROLL_LEFT:
			Roll += velocity_roll;
			break;
		case CameraMovement::ROLL_RIGHT:
			Roll -= velocity_roll;
			break;
		}
		updateCameraVectors();
	}

	void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	void Camera::processMouseScroll(float yoffset)
	{
		Fov -= yoffset;
		if (Fov < 1.0f)
			Fov = 1.0f;
		if (Fov > 179.0f)
			Fov = 179.0f;
	}

	// Captures Camera position and appends to a file called "camera.cam" (only works in debug mode)
	void Camera::capturePos()
	{
		demokernel& demo = demokernel::GetInstance();

		std::ofstream camFile;
		//string message;
		char message[1024];
		camFile.open("camera.cam", std::ios::out | std::ios::app);
		sprintf_s(message, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",
			demo.m_pCamera->Position.x, demo.m_pCamera->Position.y, demo.m_pCamera->Position.z,
			demo.m_pCamera->Up.x, demo.m_pCamera->Up.y, demo.m_pCamera->Up.z,
			demo.m_pCamera->Yaw, demo.m_pCamera->Pitch, demo.m_pCamera->Roll, demo.m_pCamera->Fov);
		camFile << message << "\n";
		camFile.close();
	}

	void Camera::setCamera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch, float roll, float fov)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Roll = roll;
		Fov = fov;
		updateCameraVectors();
	}

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void Camera::setRollMatrix(glm::mat3x3& m, glm::vec3 f) {
		float rcos = glm::cos(glm::radians(Roll));
		float rsin = glm::sin(glm::radians(Roll));

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

	void Camera::updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = glm::cos(glm::radians(Yaw)) * glm::cos(glm::radians(Pitch));
		front.y = glm::sin(glm::radians(Pitch));
		front.z = glm::sin(glm::radians(Yaw)) * glm::cos(glm::radians(Pitch));
		Front = glm::normalize(front);

		//	control the rotate view
		glm::mat3x3 rollMatrix;
		setRollMatrix(rollMatrix, front);

		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, rollMatrix * WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
}