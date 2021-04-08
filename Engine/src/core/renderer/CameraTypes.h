﻿// CameraTypes.h
// Spontz Demogroup

#pragma once

#include "main.h"

namespace Phoenix {

	// Projection Free Camera: Like an "FPS" camera: The target is not locked, the target is changed with the Euler Angles
	class CameraProjectionFree final : public Camera
	{
	public:

		CameraProjectionFree(glm::vec3 const& position);

		const glm::mat4 getProjection();
		const glm::mat4 getView();

		// Processes input received from any keyboard-like input system
		// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void	processKeyboard(CameraMovement direction, float deltaTime);
		// Processes input received from a mouse input system. Expects the offset value in both the x and y direction
		void	processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void	processMouseScroll(float yoffset);
		void	multiplyMovementSpeed(float speed);
		void	divideMovementSpeed(float speed);
		void	capturePos();
		void	reset();

	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void		setRollMatrix(glm::mat3& m, glm::vec3 f);
		void		updateCameraVectors();
	};

	// Projection Target Camera: Similar to ProjectionFree camera, but target can be specified manually
	class CameraProjectionTarget final : public Camera
	{
	public:

		CameraProjectionTarget(glm::vec3 const& position, glm::vec3 const& at);

		const glm::mat4 getProjection();
		const glm::mat4 getView();

		// Processes input received from any keyboard-like input system
		// Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void	processKeyboard(CameraMovement direction, float deltaTime);
		// Processes input received from a mouse input system. Expects the offset value in both the x and y direction
		void	processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void	processMouseScroll(float yoffset);
		void	multiplyMovementSpeed(float speed);
		void	divideMovementSpeed(float speed);
		void	capturePos();
		void	reset();

	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void		setRollMatrix(glm::mat3& m, glm::vec3 f);
		void		updateCameraVectors();
	};

	// Orthogonal camera : TODO: at this moment everything is hardcoded, it needs to be configured properly
	class CameraOrthogonal final : public Camera
	{
	public:

		CameraOrthogonal();

		const glm::mat4 getProjection();
		const glm::mat4 getView();

	private:
	};

	class CameraRawMatrix final : public Camera
	{
	public:

		CameraRawMatrix(glm::mat4 const& matrix);

		const glm::mat4 getProjection();
		const glm::mat4 getView();

		void setViewMatrix(glm::mat4 const& matrix) { m_Matrix = matrix; };

	private:
		glm::mat4		m_Matrix;	// Matrix: used for scenes with a saved matrix
	};
}