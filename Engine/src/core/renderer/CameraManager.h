// CameraManager.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Camera.h"
#include "core/renderer/CameraTypes.h"

namespace Phoenix {
	
	class CameraManager {
		friend class MathDriver;
	public:

		CameraManager();
		~CameraManager();

		void init();
		void setInternalCameraAsActive();			// Sets Internal Camera as Active Camera
		void setActiveCamera(Camera* newCamera);	// Set the "newCamera" as Active camera
		void resetInternalCamera();
		void increaseInternalCameraSpeed(float value);
		void decreaseInternalCameraSpeed(float value);
		bool captureActiveCameraPosition();
		void processKeyInternalCamera(CameraMovement direction, float deltaTime);
		void processMouseMovementInternalCamera(float xOffset, float yOffset);
		void processMouseScrollInternalCamera(float yOffset);

		Camera* getActiveCamera() { return m_pActiveCamera; };					// Get the Active Camera details
		Camera* getInternalCamera() { return m_pInternalCamera; };				// Get the Internal Camera details
		const glm::mat4 getActiveProjection() { return m_pActiveCamera->getProjection(); }
		const glm::mat4 getActiveView() { return m_pActiveCamera->getView(); }

	private:
		Camera* m_pActiveCamera;		// Current Active Camera
		Camera* m_pActiveCameraExprTk;	// Current Active Camera, used in the Math driver
		Camera* m_pInternalCamera;		// Internal engine Camera, used when no camera is defined


	};
}