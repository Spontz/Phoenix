#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sEfxFader final : public Section {
	public:
		sEfxFader();
		~sEfxFader();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		SP_Shader	m_pShader;			// Fader Shader to apply
		ShaderVars* m_pVars = nullptr;	// Shader variables
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

	sEfxFader::~sEfxFader()
	{
		if (m_pVars)
			delete m_pVars;
	}

	bool sEfxFader::load()
	{
		// script validation
		if (shaderBlock.size() != 1) {
			Logger::error("EfxFader [{}]: 1 shader required", identifier);
			return false;
		}

		// Load Fader shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename);
		if (!m_pShader)
			return false;

		// render states
		render_disableDepthTest = true;

		// Configure Fader shader
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (auto& uni : shaderBlock[0]->uniform) {
			m_pVars->ReadString(uni);
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues();
		
		return !DEMO_checkGLError();
	}

	void sEfxFader::init()
	{

	}

	void sEfxFader::warmExec()
	{
		exec();
	}

	void sEfxFader::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();
		{
			m_pShader->use();
			// Set shader variables values
			m_pVars->setValues();
			// Render scene
			m_demo.m_pRes->drawQuadFS();
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sEfxFader::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		debugStatic = ss.str();
	}

	std::string sEfxFader::debug()
	{
		return debugStatic;
	}
}