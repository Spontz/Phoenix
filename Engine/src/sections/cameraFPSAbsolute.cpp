#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sCameraFPSAbsolute : public Section {

	public:
		sCameraFPSAbsolute();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		Camera*		m_pCam = nullptr;
		bool		m_bFreeCam = false;

		// Spline cam variables
		glm::vec3	m_vCamPos = { 0, 0, 0 };
		float		m_fCamYaw = 0;
		float		m_fCamPitch = 0;
		float		m_fCamRoll = 0;
		float		m_fCamFov = 0;

		// Final cam position variables
		glm::vec3	m_vCamFinalPos = { 0, 0, 0 };
		float		m_fCamFinalYaw = 0;
		float		m_fCamFinalPitch = 0;
		float		m_fCamFinalRoll = 0;
		float		m_fCamFinalFov = 0;

		MathDriver* m_pExprCamera = nullptr;	// A equation containing the calculations of the camera
	};

	// ******************************************************************

	Section* instance_cameraFPSAbsolute()
	{
		return new sCameraFPSAbsolute();
	}


	sCameraFPSAbsolute::sCameraFPSAbsolute()
	{
		type = SectionType::CameraFPSAbsolute;
	}

	bool sCameraFPSAbsolute::load()
	{
		// script validation
		if ((spline.size() != 1) || (param.size() != 1) || (strings.size() < 2)) {
			Logger::error("Camera FPS Absolute [%s]: 1 spline, 1 param and 2 strings needed", this->identifier.c_str());
			return false;
		}

		// Create the Camera class
		m_pCam = new CameraProjectionFPS(Camera::DEFAULT_CAM_POSITION);

		// Load parameters
		m_bFreeCam = static_cast<bool>(this->param[0]);

		// Load the camera splines
		for (int i = 0; i < spline.size(); i++) {
			if (this->spline[i]->load() == false) {
				Logger::error("CameraSection [%s]: Spline not loaded", identifier.c_str());
				return false;
			}
		}

		// Load the camera modifiers (based in formulas)
		m_pExprCamera = new MathDriver(this);

		for (int i = 0; i < strings.size(); i++) {
			m_pExprCamera->expression += this->strings[i];
		}

		m_pExprCamera->SymbolTable.add_variable("c_posX", m_vCamPos.x);
		m_pExprCamera->SymbolTable.add_variable("c_posY", m_vCamPos.y);
		m_pExprCamera->SymbolTable.add_variable("c_posZ", m_vCamPos.z);

		m_pExprCamera->SymbolTable.add_variable("c_yaw", m_fCamYaw);
		m_pExprCamera->SymbolTable.add_variable("c_pitch", m_fCamPitch);
		m_pExprCamera->SymbolTable.add_variable("c_roll", m_fCamRoll);
		m_pExprCamera->SymbolTable.add_variable("c_fov", m_fCamFov);

		m_pExprCamera->SymbolTable.add_variable("PosX", m_vCamFinalPos.x);
		m_pExprCamera->SymbolTable.add_variable("PosY", m_vCamFinalPos.y);
		m_pExprCamera->SymbolTable.add_variable("PosZ", m_vCamFinalPos.z);

		m_pExprCamera->SymbolTable.add_variable("Yaw", m_fCamFinalYaw);
		m_pExprCamera->SymbolTable.add_variable("Pitch", m_fCamFinalPitch);
		m_pExprCamera->SymbolTable.add_variable("Roll", m_fCamFinalRoll);
		m_pExprCamera->SymbolTable.add_variable("Fov", m_fCamFinalFov);

		if (!m_pExprCamera->compileFormula())
			return false;
		m_pExprCamera->Expression.value();

		return true;
	}

	void sCameraFPSAbsolute::init()
	{
	}

	void sCameraFPSAbsolute::exec()
	{
		// If freeCam is active, we do nothing
		if (m_bFreeCam)
			return;

		if (!this->spline.empty()) {
			ChanVec	new_pos;

			// Calculate the motion step of the first spline and set it to "new_pos"
			this->spline[0]->MotionCalcStep(new_pos, this->runTime);

			m_vCamPos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
			m_fCamYaw = new_pos[6];
			m_fCamPitch = new_pos[7];
			m_fCamRoll = new_pos[8];
			m_fCamFov = new_pos[9];

			// apply formula modifications
			m_pExprCamera->Expression.value();

			m_pCam->setPosition(m_vCamFinalPos);
			m_pCam->setUpVector(glm::vec3(new_pos[3], new_pos[4], new_pos[5]));
			m_pCam->setAngles(m_fCamFinalYaw, m_fCamFinalPitch, m_fCamFinalRoll);
			m_pCam->setFov(m_fCamFinalFov);

			DEMO->m_pActiveCamera = m_pCam;
		}
	}

	void sCameraFPSAbsolute::end()
	{
	}

	void sCameraFPSAbsolute::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Type: " << m_pCam->TypeStr << std::endl;
		ss << "Free cam: " << (m_bFreeCam ? "Yes" : "No") << std::endl;
		debugStatic = ss.str();
	}

	std::string sCameraFPSAbsolute::debug()
	{
		return debugStatic;
	}
}