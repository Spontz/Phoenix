#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sEfxFader : public Section {
	public:
		sEfxFader();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		Shader* m_pShader = nullptr;	// Fader Shader to apply
		ShaderVars* p_Vars = nullptr;	// Shader variables
	};

	// ******************************************************************

	Section* instance_efxFader()
	{
		return new sEfxFader();
	}

	sEfxFader::sEfxFader()
	{
		type = SectionType::EfxFader;
	}


	bool sEfxFader::load()
	{
		// script validation
		if (strings.size() < 1) {
			Logger::error("EfxFader [%s]: 1 shader file required", identifier.c_str());
			return false;
		}

		// Load Fader shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
		if (!m_pShader)
			return false;

		// Configure Fader shader
		m_pShader->use();
		p_Vars = new ShaderVars(this, m_pShader);
		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			p_Vars->ReadString(uniform[i].c_str());
		}

		// Set shader variables values
		p_Vars->setValues();

		return true;
	}

	void sEfxFader::init()
	{

	}

	void sEfxFader::exec()
	{
		EvalBlendingStart();
		glDisable(GL_DEPTH_TEST);
		{
			m_pShader->use();
			// Set shader variables values
			p_Vars->setValues();
			// Render scene
			m_demo.m_pRes->Draw_QuadFS();
		}
		glEnable(GL_DEPTH_TEST);
		EvalBlendingEnd();

	}

	void sEfxFader::end()
	{

	}

	void sEfxFader::loadDebugStatic()
	{
	}

	std::string sEfxFader::debug()
	{
		return debugStatic;
	}
}