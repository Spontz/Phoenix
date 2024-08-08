// CameraManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/CameraManager.h"

namespace Phoenix {

	// Init vars
	CameraManager::CameraManager()
		:
		m_pActiveCamera(nullptr),
		m_pActiveCameraExprTk(nullptr),
		m_pInternalCamera(nullptr),
		m_bCapturePosition(false)
	{		
	}

	CameraManager::~CameraManager()
	{
		if (m_pInternalCamera)
			delete m_pInternalCamera;

		if (m_pActiveCameraExprTk)
			delete m_pActiveCameraExprTk;
	}

	void CameraManager::init()
	{
		m_pInternalCamera = new CameraProjectionFPS(Camera::DEFAULT_CAM_POSITION);
		m_pActiveCameraExprTk = new CameraProjectionFPS(Camera::DEFAULT_CAM_POSITION);

		m_pActiveCamera = m_pInternalCamera;
		m_pActiveCameraExprTk->copyData(*(m_pInternalCamera));
	}

	void CameraManager::setInternalCameraAsActive()
	{
		m_pActiveCamera = m_pInternalCamera;
		m_pActiveCameraExprTk->copyData(*(m_pInternalCamera));
	}

	void CameraManager::setActiveCamera(Camera* newCamera)
	{
		m_pActiveCamera = newCamera;
		m_pActiveCameraExprTk->copyData(*(m_pActiveCamera));
	}

	void CameraManager::resetInternalCamera()
	{
		m_pInternalCamera->reset();
	}

	void CameraManager::increaseInternalCameraSpeed(float value)
	{
		m_pInternalCamera->multiplyMovementSpeed(value);
	}

	void CameraManager::decreaseInternalCameraSpeed(float value)
	{
		m_pInternalCamera->divideMovementSpeed(value);
	}

	bool CameraManager::captureActiveCameraPosition(const std::string ID)
	{
		return m_pActiveCamera->capturePos(ID);
	}

	void CameraManager::processKeyInternalCamera(CameraMovement direction, float deltaTime)
	{
		m_pInternalCamera->processKeyboard(direction, deltaTime);
	}

	void CameraManager::processMouseMovementInternalCamera(float xOffset, float yOffset)
	{
		m_pInternalCamera->processMouseMovement(xOffset, yOffset);
	}

	void CameraManager::processMouseScrollInternalCamera(float yOffset)
	{
		m_pInternalCamera->processMouseScroll(yOffset);
	}

	
}