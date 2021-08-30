#include "main.h"
#include "core/drivers/mathdriver.h"

namespace Phoenix {

	class sLight final : public Section {
	public:
		sLight();
		~sLight();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		int			m_iLightNum = 0;		// Light Number
		bool		m_bLinkPosToCamera = false;	// Link the light to the Camera Position
		bool		m_bShadowMapping = false;	// Is the light being used for shadowMapping?
		float		m_fShadowNearPlane = 0;		// shadowMapping: Near plane used in Orthographic view
		float		m_fShadowFarPlane = 100.0f;	// shadowMapping: Far plane used in Orthographic view
		float		m_fShadowSize = 10.0f;	// shadowMapping: Size of the plane used in Orthographic view
		bool		m_bDrawLight = false;	// Draw a cube representing the light: usefult for debugging
		float		m_fDrawLightSize = 1.0f;		// Size of our debug cube
		MathDriver* m_pExprLight = nullptr;	// A equation containing the calculations of the light
	};

	// ******************************************************************

	Section* instance_light()
	{
		return new sLight();
	}

	sLight::sLight()
	{
		type = SectionType::LightSec;
	}

	sLight::~sLight()
	{
		if (m_pExprLight)
			delete m_pExprLight;
	}

	std::string& replaceString(std::string& subj, std::string_view const& old_str, std::string_view const& new_str)
	{
		size_t pos;
		do {
			pos = subj.find(old_str);
			if (pos != std::string::npos)
			{
				subj.erase(pos, old_str.size());
				subj.insert(pos, new_str);
			}
		} while (std::string::npos != pos);

		return subj;
	}

	bool sLight::load()
	{
		// script validation
		if ((param.size() != 8)) {
			Logger::error("Light [{}]: 8 params needed (light Number, link to camera position, shadowMapping, near&far planes, size, DebugDraw & DebugDraw size)", identifier);
			return false;
		}

		// Load the parameters
		m_iLightNum = (int)param[0];
		if (m_iLightNum < 0 || m_iLightNum >= m_demo.m_lightManager.light.size()) {
			Logger::error("Light: The light number is not supported by the engine. Max Lights: {}", (m_demo.m_lightManager.light.size() - 1));
			return false;
		}

		// Load parameters
		m_bLinkPosToCamera = static_cast<bool>(param[1]);

		// Load the parameters for shadow mapping
		m_bShadowMapping = static_cast<bool>(param[2]);
		m_fShadowNearPlane = param[3];
		m_fShadowFarPlane = param[4];
		m_fShadowSize = param[5];
		m_bDrawLight = static_cast<bool>(param[6]);
		m_fDrawLightSize = param[7];

		Light* my_light = m_demo.m_lightManager.light[m_iLightNum];

		// Register the variables
		m_pExprLight = new MathDriver(this);
		std::string expr;
		for (int i = 0; i < strings.size(); i++)
			expr += strings[i];
		expr = replaceString(expr, "light_", "light" + std::to_string(m_iLightNum) + "_");	// Adds the name of the light that we want to modify
		m_pExprLight->expression = expr;															// Loads the expression, properly composed
		m_pExprLight->Expression.register_symbol_table(m_pExprLight->SymbolTable);
		if (!m_pExprLight->compileFormula())
			return false;
		
		return !GLDRV_checkError();
	}

	void sLight::init()
	{
	}

	void sLight::exec()
	{
		// Evaluate the expression
		m_pExprLight->Expression.value();

		Light* my_light = m_demo.m_lightManager.light[m_iLightNum];

		if (m_bLinkPosToCamera) {
			my_light->position = m_demo.m_pActiveCamera->getPosition();
			my_light->direction = m_demo.m_pActiveCamera->getPosition() + (m_demo.m_pActiveCamera->getFront() * 10.0f); // TODO: Remove this hardcode! XD
		}


		if (m_bShadowMapping) {
			// Calculate the Space Matrix of our camera for being used in other sections
			my_light->CalcSpaceMatrix(-(m_fShadowSize), m_fShadowSize, -(m_fShadowSize), m_fShadowSize, m_fShadowNearPlane, m_fShadowFarPlane);
		}

		if (m_bDrawLight) {
			my_light->draw(m_fDrawLightSize);
		}
	}

	void sLight::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Light: " << m_iLightNum << std::endl;
		ss << "ShadowMapping: " << m_bShadowMapping << " Size: " << m_fShadowSize << " Near: " << m_fShadowNearPlane << " Far: " << m_fShadowFarPlane << std::endl;
		ss << "DrawLight: " << m_bDrawLight << std::endl;
		debugStatic = ss.str();
	}

	std::string sLight::debug()
	{
		return debugStatic;
	}
}
