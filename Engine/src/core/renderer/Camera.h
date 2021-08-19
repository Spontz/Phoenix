// Camera.h
// Spontz Demogroup

#pragma once

#include "main.h"

namespace Phoenix {

	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum class CameraMovement : int32_t {
		FORWARD = 0,
		BACKWARD = 1,
		LEFT = 2,
		RIGHT = 3,
		ROLL_LEFT = 4,
		ROLL_RIGHT = 5
	};

	enum class CameraType : int32_t {
		NONE = 0,
		PROJ_FPS = 1,		// FPS like camera, only using euler angles
		PROJ_TARGET = 2,	// Target: Using a target, euler angles are also available
		RAW_MATRIX = 3,		// RAW_Matrix: Useful when a matrix is given (f.i.: when reading an animated camera from a 3d file)
		ORTHOGONAL = 4		// Orthogonal camera
	};

	class Camera
	{
		// Friend Classes
		friend class CameraProjectionFPS;
		friend class CameraProjectionTarget;
		friend class CameraRawMatrix;
		friend class MathDriver;

	public:
		// Default camera values
		static constexpr glm::vec3 DEFAULT_CAM_POSITION{ 0, 0, 3 };
		static constexpr glm::vec3 DEFAULT_CAM_TARGET{ 0, 0, 0 };
		static constexpr glm::vec3 DEFAULT_CAM_UP{ 0, 1, 0 };
		static constexpr glm::vec3 DEFAULT_CAM_RIGHT{ 1, 0, 0 };
		static constexpr glm::vec3 DEFAULT_CAM_WORLD_UP{ 0, 1, 0 };
		static constexpr glm::vec3 DEFAULT_CAM_FRONT{ 0, 0, 0 };
		static constexpr float DEFAULT_CAM_YAW = -90.0f;
		static constexpr float DEFAULT_CAM_PITCH = 0.0f;
		static constexpr float DEFAULT_CAM_ROLL = 0.0f;
		static constexpr float DEFAULT_CAM_FOV = 45.0f;
		static constexpr float DEFAULT_CAM_MOVEMENT_SPEED = 15.0f;
		static constexpr float DEFAULT_CAM_ROLL_SPEED = 40.0f;
		static constexpr float DEFAULT_CAM_SENSITIVITY = 0.1f;
		static constexpr float DEFAULT_CAM_NEAR = 0.1f;
		static constexpr float DEFAULT_CAM_FAR = 10000.0f;

	public:
		CameraType		Type;
		std::string		TypeStr;

	public:
		Camera();
		virtual ~Camera() = default;

		virtual const glm::mat4 getProjection() { return glm::mat4(1.0f); };
		virtual const glm::mat4 getView() { return glm::mat4(1.0f); };
		
		const glm::vec3& getPosition() { return m_Position; };
		const glm::vec3& getTarget() { return m_Target; };
		const glm::vec3& getFront() { return m_Front; };

		float getMovementSpeed() const { return m_MovementSpeed; };
		float getYaw() const { return m_Yaw; };
		float getPitch() const { return m_Pitch; };
		float getRoll() const { return m_Roll; };
		float getFov() const { return m_Fov; };
		float getFrustumNear() const { return m_FrustumNear; };
		float getFrustumFar() const { return m_FrustumFar; };

		virtual void processKeyboard(CameraMovement direction, float deltaTime) {};
		virtual void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {};
		virtual void processMouseScroll(float yoffset) {};
		virtual void multiplyMovementSpeed(float speed) {};
		virtual void divideMovementSpeed(float speed) {};
		virtual bool capturePos() { return false; };
		virtual void reset() {};

		virtual void setViewMatrix(glm::mat4 const& matrix) {};
		virtual void setPosition(glm::vec3 const& position) { m_Position = position; };
		virtual void setTarget(glm::vec3 const& target) { m_Target = target; };
		virtual void setUpVector(glm::vec3 const& up) { m_Up = up; };
		virtual void setFov(float fov) { m_Fov = fov; };
		virtual void setAngles(float yaw, float pitch, float roll);
		virtual void setFrustum(float frustum_near, float frustum_far);


	private:
		glm::vec3				m_Position = DEFAULT_CAM_POSITION;
		glm::vec3				m_Target = DEFAULT_CAM_TARGET;
		glm::vec3				m_Up = DEFAULT_CAM_UP;
		glm::vec3				m_Right = DEFAULT_CAM_RIGHT;
		mutable glm::vec3		m_Front = DEFAULT_CAM_FRONT; // TODO: To be changed by a function

		// Euler angles
		float m_Yaw = DEFAULT_CAM_YAW;
		float m_Pitch = DEFAULT_CAM_PITCH;
		float m_Roll = DEFAULT_CAM_ROLL;
		float m_Fov = DEFAULT_CAM_FOV;

		// Camera behaviour
		float m_MovementSpeed = DEFAULT_CAM_MOVEMENT_SPEED;
		float m_RollSpeed = DEFAULT_CAM_ROLL_SPEED;
		float m_MouseSensitivity = DEFAULT_CAM_SENSITIVITY;

		// Frustum
		float m_FrustumNear = DEFAULT_CAM_NEAR;
		float m_FrustumFar = DEFAULT_CAM_FAR;
	};
}
