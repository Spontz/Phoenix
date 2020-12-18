// Camera.h
// Spontz Demogroup

// Right Handed. Degrees. Column Major.

#pragma once

#include "main.h"

namespace Phoenix {

	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum class CameraMovement : int {
		FORWARD = 0,
		BACKWARD = 1,
		LEFT = 2,
		RIGHT = 3,
		ROLL_LEFT = 4,
		ROLL_RIGHT = 5
	};

	// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
	class Camera
	{
		// Default camera values
		static const float DEFAULT_CAM_YAW;
		static const float DEFAULT_CAM_PITCH;
		static const float DEFAULT_CAM_SPEED;
		static const float DEFAULT_CAM_ROLL_SPEED;
		static const float DEFAULT_CAM_SENSITIVITY;
		static const float DEFAULT_CAM_VFOV;
		static const float DEFAULT_CAM_ROLL;

	public:
		std::string		Name;
		// Camera Attributes
		glm::vec3		Position;
		glm::vec3		Front;
		glm::vec3		Up;
		glm::vec3		Right;
		glm::vec3		WorldUp;
		glm::mat4		Matrix;

		// Euler Angles
		float Yaw;
		float Pitch;
		float Roll;

		// Camera options
		float MovementSpeed;
		float RollSpeed;
		float MouseSensitivity;
		float Zoom;

		// Constructor with vectors
		Camera(
			glm::vec3 const& position = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 const& up = glm::vec3(0.0f, 1.0f, 0.0f),
			float yaw = DEFAULT_CAM_YAW,
			float pitch = DEFAULT_CAM_PITCH,
			float roll = DEFAULT_CAM_ROLL
		);

		void		Reset();
		// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4	GetViewMatrix() const;
		glm::mat4	GetMatrix() const;
		// Processes input received from any keyboard-like input system
		// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void		ProcessKeyboard(CameraMovement direction, float deltaTime);
		// Processes input received from a mouse input system. Expects the offset value in both the x and y direction
		void		ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
		// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void		ProcessMouseScroll(float yoffset);
		void		CapturePos();
		void		setCamera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch, float roll, float zoom);
		glm::mat4	getOrthoMatrix_Projection() const;
		glm::mat4	getOrthoMatrix_View() const;

	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void		setRollMatrix(glm::mat3x3& m, glm::vec3 f);
		void		updateCameraVectors();
	};
}