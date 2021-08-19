// CameraTypes.h
// Spontz Demogroup

#pragma once

#include "main.h"

namespace Phoenix {

	// Projection FPS Camera: Like an "FPS" camera: The target is not locked, the target is changed
	// with the Euler Angles
	class CameraProjectionFPS final : public Camera
	{
	public:
		CameraProjectionFPS(glm::vec3 const& position);

	public:
		const glm::mat4 getProjection() override;
		const glm::mat4 getView() override;

		// Processes input received from any keyboard-like input system
		// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing
		// systems)
		void processKeyboard(CameraMovement direction, float deltaTime) override;
		// Processes input received from a mouse input system. Expects the offset value in both the
		// x and y direction
		void processMouseMovement(
			float xOffset,
			float yOffset,
			bool constrainPitch = true
		) override;
		// Processes input received from a mouse scroll-wheel event. Only requires input on the
		// vertical wheel-axis
		void processMouseScroll(float yoffset) override;
		void multiplyMovementSpeed(float speed) override;
		void divideMovementSpeed(float speed) override;
		bool capturePos() override;
		void reset() override;

	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void setRollMatrix(glm::mat3& m, glm::vec3 const& f);
		void updateCameraVectors();
	};

	// Projection Target Camera: Similar to ProjectionFree camera, but target can be specified
	// manually
	class CameraProjectionTarget final : public Camera
	{
	public:
		CameraProjectionTarget(glm::vec3 const& position, glm::vec3 const& target);

	public:
		const glm::mat4 getProjection() override;
		const glm::mat4 getView() override;

		// Processes input received from any keyboard-like input system
		// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing
		// systems)
		void processKeyboard(CameraMovement direction, float deltaTime) override;
		// Processes input received from a mouse input system. Expects the offset value in both the
		// x and y direction
		void processMouseMovement(
			float xOffset,
			float yOffset,
			bool constrainPitch = true
		) override;
		// Processes input received from a mouse scroll-wheel event. Only requires input on the
		// vertical wheel-axis
		void processMouseScroll(float yoffset) override;
		void multiplyMovementSpeed(float speed) override;
		void divideMovementSpeed(float speed) override;
		bool capturePos() override;
		void reset() override;
	};

	// Orthogonal camera : TODO: at this moment everything is hardcoded, it needs to be configured
	// properly
	class CameraOrthogonal final : public Camera
	{
	public:
		CameraOrthogonal();

	public:
		const glm::mat4 getProjection() override;
		const glm::mat4 getView() override;
	};

	class CameraRawMatrix final : public Camera
	{
	public:
		CameraRawMatrix(glm::mat4 const& matrix);

	public:
		const glm::mat4 getProjection() override;
		const glm::mat4 getView() override;

		void setViewMatrix(glm::mat4 const& matrix) override { m_Matrix = matrix; };

	private:
		glm::mat4 m_Matrix; // Matrix: used for scenes with a saved matrix
	};
}
