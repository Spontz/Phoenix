#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawQuad final : public Section {
	public:
		sDrawQuad();
		~sDrawQuad();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		SP_Shader		m_pShader;	// Shader to apply
		ShaderVars*		m_pVars = nullptr;	// Shader variables
	};

	// ******************************************************************

	Section* instance_drawQuad()
	{
		return new sDrawQuad();
	}

	sDrawQuad::sDrawQuad()
	{
		type = SectionType::DrawQuad;
	}

	sDrawQuad::~sDrawQuad()
	{
		if (m_pVars)
			delete m_pVars;
	}

	bool sDrawQuad::load()
	{
		// script validation
		if ((param.size()) != 2 || (strings.size() != 1)) {
			Logger::error("DrawQuad [%s]: 2 params are needed (Clear the screen buffer & clear depth buffer), and the shader file", identifier.c_str());
			return false;
		}

		render_disableDepthTest = true;
		// Load parameters
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);

		// Load shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
		if (!m_pShader)
			return false;

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}

		// Set shader variables values
		m_pVars->setValues();

		return true;
	}

	void sDrawQuad::init()
	{

	}

	void sDrawQuad::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();
		{
			m_pShader->use();
			m_pVars->setValues();
			m_demo.m_pRes->drawQuadFS();
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawQuad::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawQuad::debug()
	{
		return "";
	}
}