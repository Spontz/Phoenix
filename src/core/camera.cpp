// camera.cpp
// Spontz Demogroup

#include "main.h"
#include "core/camera.h"

// Default camera values
const float Camera::DEFAULT_CAM_YAW = -90.0f;
const float Camera::DEFAULT_CAM_PITCH = 0.0f;
const float Camera::DEFAULT_CAM_SPEED = 15.0f;
const float Camera::DEFAULT_CAM_SENSITIVITY = 0.1f;
const float Camera::DEFAULT_CAM_VFOV = 45.0f;

Camera::Camera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch)
{
	Name = "Default";
	Front = glm::vec3(0.0f, 0.0f, -1.0f);
	MovementSpeed = DEFAULT_CAM_SPEED;
	MouseSensitivity = DEFAULT_CAM_SENSITIVITY;
	Zoom = DEFAULT_CAM_VFOV;

	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

void Camera::Reset()
{
	Front = glm::vec3(0.0f, 0.0f, -1.0f);
	MovementSpeed = DEFAULT_CAM_SPEED;
	MouseSensitivity = DEFAULT_CAM_SENSITIVITY;
	Zoom = DEFAULT_CAM_VFOV;

	Position = glm::vec3(0.0f, 0.0f, 3.0f);
	WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	Yaw = DEFAULT_CAM_YAW;
	Pitch = DEFAULT_CAM_PITCH;
	updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetMatrix() const
{
	return Matrix;
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	const float velocity = MovementSpeed * deltaTime;

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
	}
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
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

void Camera::ProcessMouseScroll(float yoffset)
{
	if (Zoom >= 1.0f && Zoom <= 45.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 45.0f)
		Zoom = 45.0f;
}

// Captures Camera position and appends to a file called "camera.cam" (only works in debug mode)
void Camera::CapturePos()
{
	ofstream camFile;
	//string message;
	char message[1024];
	camFile.open("camera.cam", ios::out | ios::app);
	sprintf_s(message, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",
		DEMO->camera->Position.x, DEMO->camera->Position.y, DEMO->camera->Position.z,
		DEMO->camera->Up.x, DEMO->camera->Up.y, DEMO->camera->Up.z,
		DEMO->camera->Yaw, DEMO->camera->Pitch, DEMO->camera->Zoom);
	camFile << message << "\n";
	camFile.close();
}

void Camera::setCamera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch, float zoom)
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	Zoom = zoom;
	updateCameraVectors();
}

glm::mat4 Camera::getOrthoMatrix_Projection() const
{
	return glm::ortho(-1, 1, 1, -1, -1, 2);
}

glm::mat4 Camera::getOrthoMatrix_View() const
{
	return glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}
