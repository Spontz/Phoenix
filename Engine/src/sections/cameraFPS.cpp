#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sCameraFPS final : public Section {

	// Defines the camera mode
	enum class CameraMode : int {
		FREE = 0,
		ONLY_SPLINE = 1,
		ONLY_FORMULA = 2,
		SPLINE_AND_FORMULA = 3
	};

	public:
		sCameraFPS();
		~sCameraFPS();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		Camera*		m_pCam = nullptr;
		CameraMode	m_iCameraMode = CameraMode::FREE;

		// Spline cam variables
		glm::vec3	m_vCamPos = CameraProjectionFPS::DEFAULT_CAM_POSITION;
		glm::vec3	m_vCamUp = CameraProjectionFPS::DEFAULT_CAM_UP;
		float		m_fCamYaw = CameraProjectionFPS::DEFAULT_CAM_YAW;
		float		m_fCamPitch = CameraProjectionFPS::DEFAULT_CAM_PITCH;
		float		m_fCamRoll = CameraProjectionFPS::DEFAULT_CAM_ROLL;
		float		m_fCamFov = CameraProjectionFPS::DEFAULT_CAM_FOV;
		float		m_fCamFrustumNear = CameraProjectionFPS::DEFAULT_CAM_NEAR;
		float		m_fCamFrustumFar = CameraProjectionFPS::DEFAULT_CAM_FAR;

		// Final cam position variables
		glm::vec3	m_vCamFinalPos = CameraProjectionFPS::DEFAULT_CAM_POSITION;
		glm::vec3	m_vCamFinalUp = CameraProjectionFPS::DEFAULT_CAM_UP;
		float		m_fCamFinalYaw = CameraProjectionFPS::DEFAULT_CAM_YAW;
		float		m_fCamFinalPitch = CameraProjectionFPS::DEFAULT_CAM_PITCH;
		float		m_fCamFinalRoll = CameraProjectionFPS::DEFAULT_CAM_ROLL;
		float		m_fCamFinalFov = CameraProjectionFPS::DEFAULT_CAM_FOV;
		float		m_fCamFinalFrustumNear = CameraProjectionFPS::DEFAULT_CAM_NEAR;
		float		m_fCamFinalFrustumFar = CameraProjectionFPS::DEFAULT_CAM_FAR;


		MathDriver* m_pExprCamera = nullptr;	// An equation containing the calculations of the camera
	};

	// ******************************************************************

	Section* instance_cameraFPS()
	{
		return new sCameraFPS();
	}

	sCameraFPS::sCameraFPS()
	{
		type = SectionType::CameraFPS;
	}

	sCameraFPS::~sCameraFPS()
	{
		if (m_pExprCamera)
			delete m_pExprCamera;
		if (m_pCam)
			delete m_pCam;
	}

	bool sCameraFPS::load()
	{
		// script validation
		if ((param.size() != 1)) {
			Logger::error("Camera FPS [{}]: 1 param needed (the camera mode has not been defined)", identifier);
			return false;
		}

		// Load parameter: Camera mode
		m_iCameraMode = static_cast<CameraMode>(param[0]);

		// Do validations
		switch (m_iCameraMode)
		{
		case CameraMode::FREE:
			break;
		case CameraMode::ONLY_SPLINE:
			if (spline.size() == 0) {
				Logger::error("Camera FPS [{}]: Spline mode: a spline file is needed (camera file)", identifier);
				return false;
			}
			break;
		case CameraMode::ONLY_FORMULA:
			if (strings.size() == 0) {
				Logger::error("Camera FPS [{}]: Formula mode: strings are needed with the formulas", identifier);
				return false;
			}
			break;
		case CameraMode::SPLINE_AND_FORMULA:
			if ((spline.size() == 0) || (strings.size() == 0)) {
				Logger::error("Camera FPS [{}]: Spline and Formula mode: spline and strings are needed", identifier);
				return false;
			}
			break;
		default:
			Logger::error("Camera FPS [{}]: Camera mode not supported", identifier);
			return false;
			break;
		}

		// Load the camera splines (if any)
		for (int i = 0; i < spline.size(); i++) {
			if (spline[i]->load() == false) {
				Logger::error("Camera FPS [{}]: Spline not loaded", identifier);
				return false;
			}
		}

		// Load the camera modifiers (based in formulas)
		m_pExprCamera = new MathDriver(this);

		for (int i = 0; i < strings.size(); i++) {
			m_pExprCamera->expression += strings[i];
		}

		// Camera (spline) variables
		m_pExprCamera->SymbolTable.add_variable("cPosX", m_vCamPos.x);
		m_pExprCamera->SymbolTable.add_variable("cPosY", m_vCamPos.y);
		m_pExprCamera->SymbolTable.add_variable("cPosZ", m_vCamPos.z);

		m_pExprCamera->SymbolTable.add_variable("cUpX", m_vCamUp.x);
		m_pExprCamera->SymbolTable.add_variable("cUpY", m_vCamUp.y);
		m_pExprCamera->SymbolTable.add_variable("cUpZ", m_vCamUp.z);

		m_pExprCamera->SymbolTable.add_variable("cYaw", m_fCamYaw);
		m_pExprCamera->SymbolTable.add_variable("cPitch", m_fCamPitch);
		m_pExprCamera->SymbolTable.add_variable("cRoll", m_fCamRoll);
		m_pExprCamera->SymbolTable.add_variable("cFov", m_fCamFov);

		m_pExprCamera->SymbolTable.add_variable("cNear", m_fCamFrustumNear);
		m_pExprCamera->SymbolTable.add_variable("cFar", m_fCamFrustumFar);

		// Final camera position variables
		m_pExprCamera->SymbolTable.add_variable("PosX", m_vCamFinalPos.x);
		m_pExprCamera->SymbolTable.add_variable("PosY", m_vCamFinalPos.y);
		m_pExprCamera->SymbolTable.add_variable("PosZ", m_vCamFinalPos.z);

		m_pExprCamera->SymbolTable.add_variable("UpX", m_vCamFinalUp.x);
		m_pExprCamera->SymbolTable.add_variable("UpY", m_vCamFinalUp.y);
		m_pExprCamera->SymbolTable.add_variable("UpZ", m_vCamFinalUp.z);

		m_pExprCamera->SymbolTable.add_variable("Yaw", m_fCamFinalYaw);
		m_pExprCamera->SymbolTable.add_variable("Pitch", m_fCamFinalPitch);
		m_pExprCamera->SymbolTable.add_variable("Roll", m_fCamFinalRoll);
		m_pExprCamera->SymbolTable.add_variable("Fov", m_fCamFinalFov);

		m_pExprCamera->SymbolTable.add_variable("Near", m_fCamFinalFrustumNear);
		m_pExprCamera->SymbolTable.add_variable("Far", m_fCamFinalFrustumFar);

		if (!m_pExprCamera->compileFormula())
			return false;
		m_pExprCamera->Expression.value();

		// Create the Camera class
		m_pCam = new CameraProjectionFPS(CameraProjectionFPS::DEFAULT_CAM_POSITION);

		return !DEMO_checkGLError();
	}

	void sCameraFPS::init()
	{
	}

	void sCameraFPS::exec()
	{
		ChanVec	new_pos;

		switch (m_iCameraMode)
		{
		case CameraMode::FREE:				// FREE camera: Do nothing
			m_demo.m_cameraManager.setInternalCameraAsActive();
			return;
			break;
		case CameraMode::ONLY_SPLINE:		// Only spline: Do not use formulas
			// Calculate the motion step of the first spline and set it to "new_pos"
			spline[0]->MotionCalcStep(new_pos, runTime);

			m_vCamFinalPos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
			m_vCamFinalUp = glm::vec3(new_pos[3], new_pos[4], new_pos[5]);
			m_fCamFinalYaw = new_pos[6];
			m_fCamFinalPitch = new_pos[7];
			m_fCamFinalRoll = new_pos[8];
			m_fCamFinalFov = new_pos[9];
			m_fCamFinalFrustumNear = new_pos[10];
			m_fCamFinalFrustumFar = new_pos[11];
			break;
		case CameraMode::ONLY_FORMULA:		// Only Formulas: Do not use spline file
			// Evaluate formulas
			m_pExprCamera->Expression.value();
			break;
		case CameraMode::SPLINE_AND_FORMULA:// Spline and formula: Combine both
			// Calculate the motion step of the first spline and set it to "new_pos"
			spline[0]->MotionCalcStep(new_pos, runTime);

			m_vCamPos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
			m_vCamUp = glm::vec3(new_pos[3], new_pos[4], new_pos[5]);
			m_fCamYaw = new_pos[6];
			m_fCamPitch = new_pos[7];
			m_fCamRoll = new_pos[8];
			m_fCamFov = new_pos[9];
			m_fCamFrustumNear = new_pos[10];
			m_fCamFrustumFar = new_pos[11];
			// Evaluate formulas
			m_pExprCamera->Expression.value();
			break;
		}

		m_pCam->setPosition(m_vCamFinalPos);
		m_pCam->setUpVector(m_vCamFinalUp);
		m_pCam->setAngles(m_fCamFinalYaw, m_fCamFinalPitch, m_fCamFinalRoll);
		m_pCam->setFov(m_fCamFinalFov);
		m_pCam->setFrustum(m_fCamFinalFrustumNear, m_fCamFinalFrustumFar);

		m_demo.m_cameraManager.setActiveCamera(m_pCam);
	}

	void sCameraFPS::loadDebugStatic()
	{
		std::string sCameraMode;
		switch (m_iCameraMode)
		{
		case CameraMode::FREE:
			sCameraMode = "Free (Using internal camera)";
			break;
		case CameraMode::ONLY_SPLINE:
			sCameraMode = "Only Spline";
			break;
		case CameraMode::ONLY_FORMULA:
			sCameraMode = "Only Formulas";
			break;
		case CameraMode::SPLINE_AND_FORMULA:
			sCameraMode = "Spline and Formulas";
			break;
		default:
			sCameraMode = "Mode not supported";
			break;
		}

		std::stringstream ss;
		ss << "Type: " << m_pCam->TypeStr << std::endl;
		ss << "Camera Mode: " << sCameraMode << std::endl;
		debugStatic = ss.str();
	}

	std::string sCameraFPS::debug()
	{
		return debugStatic;
	}
}
