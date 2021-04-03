#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sCameraTarget : public Section {

	public:
		sCameraTarget();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		bool		m_bFreeCam = false;

		glm::vec3	m_vCamPos = { 0, 0, 0 };
		glm::vec3	m_vCamAt = { 0, 0, 1 };
		glm::vec3	m_vCamUp = { 0, 1, 0 };

		float		m_fCamYaw = 0;
		float		m_fCamPitch = 0;
		float		m_fCamRoll = 0;
		float		m_fCamFov = 0;

		MathDriver* m_pExprCamera = nullptr;	// A equation containing the calculations of the camera
	};

	// ******************************************************************

	Section* instance_cameraTarget()
	{
		return new sCameraTarget();
	}


	sCameraTarget::sCameraTarget()
	{
		type = SectionType::CameraTarget;
	}

	bool sCameraTarget::load()
	{
		// script validation
		if ((param.size() != 1) || (strings.size() < 2)) {
			Logger::error("Camera Target [%s]: 1 param and 4 strings needed", this->identifier.c_str());
			return false;
		}

		// Load parameters
		m_bFreeCam = static_cast<bool>(this->param[0]);

		// Load the camera modifiers (based in formulas)
		m_pExprCamera = new MathDriver(this);

		for (int i = 0; i < strings.size(); i++) {
			m_pExprCamera->expression += this->strings[i];
		}

		m_pExprCamera->SymbolTable.add_variable("PosX", m_vCamPos.x);
		m_pExprCamera->SymbolTable.add_variable("PosY", m_vCamPos.y);
		m_pExprCamera->SymbolTable.add_variable("PosZ", m_vCamPos.z);

		m_pExprCamera->SymbolTable.add_variable("AtX", m_vCamAt.x);
		m_pExprCamera->SymbolTable.add_variable("AtY", m_vCamAt.y);
		m_pExprCamera->SymbolTable.add_variable("AtZ", m_vCamAt.z);

		m_pExprCamera->SymbolTable.add_variable("UpX", m_vCamUp.x);
		m_pExprCamera->SymbolTable.add_variable("UpY", m_vCamUp.y);
		m_pExprCamera->SymbolTable.add_variable("UpZ", m_vCamUp.z);

		m_pExprCamera->SymbolTable.add_variable("Yaw", m_fCamYaw);
		m_pExprCamera->SymbolTable.add_variable("Pitch", m_fCamPitch);
		m_pExprCamera->SymbolTable.add_variable("Roll", m_fCamRoll);
		m_pExprCamera->SymbolTable.add_variable("Fov", m_fCamFov);

		if (!m_pExprCamera->compileFormula())
			return false;
		m_pExprCamera->Expression.value();

		return true;
	}

	void sCameraTarget::init()
	{
	}

	void sCameraTarget::exec()
	{
		// If freeCam is active, we do nothing
		if (m_bFreeCam)
			return;

		// apply formula modifications
		m_pExprCamera->Expression.value();

		*DEMO->m_pCamera = Camera(m_vCamPos, m_vCamAt, m_vCamUp, m_fCamYaw, m_fCamPitch, m_fCamRoll);

		//DEMO->m_pCamera->setCamera(m_vCamPos, forward, m_vCamUp, m_fCamFov);
	
	}

	void sCameraTarget::end()
	{
	}

	void sCameraTarget::loadDebugStatic()
	{
	}

	std::string sCameraTarget::debug()
	{
		return "";
	}
}