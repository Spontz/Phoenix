#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sDrawQuad : public Section {
	public:
		sDrawQuad();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		Shader*		m_pShader = nullptr;	// Shader to apply
		ShaderVars*	m_pVars = nullptr;	// Shader variables
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


	bool sDrawQuad::load()
	{
		// script validation
		if ((param.size()) != 2 || (strings.size() != 1)) {
			Logger::error("DrawQuad [%s]: 2 params are needed (Clear the screen buffer & clear depth buffer), and the shader file", identifier.c_str());
			return false;
		}

		render_enableDepthTest = false;
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
			m_demo.m_pRes->Draw_QuadFS();
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawQuad::end()
	{

	}

	void sDrawQuad::loadDebugStatic()
	{
	}

	std::string sDrawQuad::debug()
	{
		return "";
	}
}