// camera.cpp
// Spontz Demogroup

#include "main.h"
#include "core/camera.h"

// Default camera values
const float Camera::DEFAULT_CAM_YAW = -90.0f;
const float Camera::DEFAULT_CAM_PITCH = 0.0f;
const float Camera::DEFAULT_CAM_ROLL = 0.0f;
const float Camera::DEFAULT_CAM_SPEED = 15.0f;
const float Camera::DEFAULT_CAM_ROLL_SPEED = 40.0f;
const float Camera::DEFAULT_CAM_SENSITIVITY = 0.1f;
const float Camera::DEFAULT_CAM_VFOV = 45.0f;

Camera::Camera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch, float roll)
{
	Name = "Default";
	Front = glm::vec3(0.0f, 0.0f, -1.0f);
	MovementSpeed = DEFAULT_CAM_SPEED;
	RollSpeed = DEFAULT_CAM_ROLL_SPEED;
	MouseSensitivity = DEFAULT_CAM_SENSITIVITY;
	Zoom = DEFAULT_CAM_VFOV;

	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	Roll = roll;
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
	Roll = DEFAULT_CAM_ROLL;
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
	if (Zoom >= 1.0f && Zoom <= 179.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 179.0f)
		Zoom = 179.0f;
}

// Captures Camera position and appends to a file called "camera.cam" (only works in debug mode)
void Camera::CapturePos()
{
	demokernel& demo = demokernel::GetInstance();

	std::ofstream camFile;
	//string message;
	char message[1024];
	camFile.open("camera.cam", std::ios::out | std::ios::app);
	sprintf_s(message, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",
		demo.camera->Position.x, demo.camera->Position.y, demo.camera->Position.z,
		demo.camera->Up.x, demo.camera->Up.y, demo.camera->Up.z,
		demo.camera->Yaw, demo.camera->Pitch, demo.camera->Zoom, demo.camera->Roll);
	camFile << message << "\n";
	camFile.close();
}

void Camera::setCamera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch, float roll, float zoom)
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	Roll = roll;
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

// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::setRollMatrix(glm::mat3x3& m, glm::vec3 f) {
	float rcos = cos(glm::radians(Roll));
	float rsin = sin(glm::radians(Roll));

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
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);

	//	control the rotate view
	glm::mat3x3 rollMatrix;
	setRollMatrix(rollMatrix, front);

	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, rollMatrix * WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}
