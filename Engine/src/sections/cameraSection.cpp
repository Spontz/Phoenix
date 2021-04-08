#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sCamera : public Section {
	public:
		sCamera();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		Camera*		m_pCam = nullptr;
		bool		m_bFreeCam = false;
		glm::vec3	m_vCamPos = { 0, 0, 0 };
		float		m_fCamYaw = 0;
		float		m_fCamPitch = 0;
		float		m_fCamRoll = 0;
		float		m_fCamFov = 0;

		MathDriver* m_pExprCamera = nullptr;	// A equation containing the calculations of the camera
	} camera_section;

	// ******************************************************************

	Section* instance_camera()
	{
		return new sCamera();
	}
	sCamera::sCamera()
	{
		type = SectionType::CameraSec;
	}

	bool sCamera::load()
	{
		// script validation
		if ((spline.size() != 1) || (param.size() != 1)) {
			Logger::error("Camera [%s]: 1 spline and 1 param needed", identifier.c_str());
			return false;
		}

		// TODO: To be able to define the camera type?
		m_pCam = new CameraProjectionFree(Camera::DEFAULT_CAM_POSITION);

		// Load parameters
		m_bFreeCam = static_cast<bool>(param[0]);


		// Load the camera splines
		for (int i = 0; i < spline.size(); i++) {
			if (spline[i]->load() == false) {
				Logger::error("CameraSection [%s]: Spline not loaded", identifier.c_str());
				return false;
			}
		}

		// Load the camera modifiers (based in formulas)
		m_pExprCamera = new MathDriver(this);
		m_pExprCamera->expression += "var zzz:=0;"; // Hack: We set up a basic string, just in case no string is given

		for (int i = 0; i < strings.size(); i++) {
			m_pExprCamera->expression += strings[i];
		}

		m_pExprCamera->SymbolTable.add_variable("PosX", m_vCamPos.x);
		m_pExprCamera->SymbolTable.add_variable("PosY", m_vCamPos.y);
		m_pExprCamera->SymbolTable.add_variable("PosZ", m_vCamPos.z);

		m_pExprCamera->SymbolTable.add_variable("Yaw", m_fCamYaw);
		m_pExprCamera->SymbolTable.add_variable("Pitch", m_fCamPitch);
		m_pExprCamera->SymbolTable.add_variable("Roll", m_fCamRoll);
		m_pExprCamera->SymbolTable.add_variable("Fov", m_fCamFov);


		if (!m_pExprCamera->compileFormula())
			return false;
		m_pExprCamera->Expression.value();

		return true;
	}

	void sCamera::init()
	{
	}

	void sCamera::exec()
	{
		// If freeCam is active, we do nothing
		if (m_bFreeCam)
			return;

		if (!spline.empty()) {
			ChanVec	new_pos;

			// Calculate the motion step of the first spline and set it to "new_pos"
			spline[0]->MotionCalcStep(new_pos, runTime);

			// apply formula modifications
			m_pExprCamera->Expression.value();
			
			glm::vec3 Position = glm::vec3(new_pos[0], new_pos[1], new_pos[2]) + m_vCamPos;
			m_pCam->setPosition(Position);
			m_pCam->setUpVector(glm::vec3(new_pos[3], new_pos[4], new_pos[5]));
			m_pCam->setAngles(new_pos[6] + m_fCamYaw, new_pos[7] + m_fCamPitch, new_pos[8] + m_fCamRoll);
			m_pCam->setFov(new_pos[9] + m_fCamFov);

			DEMO->m_pActiveCamera = m_pCam;
		}
	}

	void sCamera::end()
	{
	}

	void sCamera::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Type: " << m_pCam->TypeStr << std::endl;
		ss << "Free cam: " << (m_bFreeCam ? "Yes":"No") << std::endl;
		debugStatic = ss.str();
	}

	std::string sCamera::debug()
	{
		return debugStatic;
	}
}