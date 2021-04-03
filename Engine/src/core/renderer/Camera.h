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

	enum class CameraType : int {
		FREE = 0,
		TARGET = 1,
		RAW_MATRIX = 2
	};

	// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
	class Camera
	{
	public:
		// Default camera values
		static constexpr glm::vec3 DEFAULT_POSITION = {0,0,3};
		static constexpr float DEFAULT_CAM_YAW = -90.0f;
		static constexpr float DEFAULT_CAM_PITCH = 0.0f;
		static constexpr float DEFAULT_CAM_ROLL = 0.0f;
		static constexpr float DEFAULT_CAM_SPEED = 15.0f;
		static constexpr float DEFAULT_CAM_ROLL_SPEED = 40.0f;
		static constexpr float DEFAULT_CAM_SENSITIVITY = 0.1f;
		static constexpr float DEFAULT_CAM_VFOV = 45.0f;
		static constexpr float DEFAULT_CAM_NEAR = 0.1f;
		static constexpr float DEFAULT_CAM_FAR = 10000.0f;

	public:
		CameraType		camType;

		std::string		Name;
		// Camera Attributes
		glm::vec3		Position;
		glm::vec3		At;
		glm::vec3		Up;
		glm::vec3		Right;
		glm::vec3		WorldUp;
		glm::mat4		Matrix;	// Matrix: used for scenes with a saved matrix

		mutable glm::vec3		Front;	// TODO: To be changed by a function
				
		// Euler Angles
		float Yaw;
		float Pitch;
		float Roll;

		// Camera options
		float MovementSpeed;
		float RollSpeed;
		float MouseSensitivity;
		float Fov;

		// Camera frustrum
		float Near;
		float Far;

		// Free camera
		Camera(glm::vec3 const& position, float yaw, float pitch, float roll);
		// Target + Free camera
		Camera(
			glm::vec3 const& position,
			glm::vec3 const& at,
			glm::vec3 const& up = glm::vec3(0.0f, 1.0f, 0.0f),
			float yaw = DEFAULT_CAM_YAW,
			float pitch = DEFAULT_CAM_PITCH,
			float roll = DEFAULT_CAM_ROLL
		);
		// Raw Matrix
		Camera(glm::mat4 const& matrix);


		void		reset();
		// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4	getViewMatrix() const;
		glm::mat4	getProjectionMatrix() const;
		glm::mat4	getOrthoProjectionMatrix() const;
		glm::mat4	getOrthoViewMatrix() const;
		glm::mat4	getMatrix() const;

		// Processes input received from any keyboard-like input system
		// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void		processKeyboard(CameraMovement direction, float deltaTime);
		// Processes input received from a mouse input system. Expects the offset value in both the x and y direction
		void		processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void		processMouseScroll(float yoffset);
		void		capturePos();
		void		setCamera(glm::vec3 const& position, glm::vec3 const& up, float yaw, float pitch, float roll, float fov);
		
	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void		setRollMatrix(glm::mat3x3& m, glm::vec3 f);
		void		updateCameraVectors();
	};
}