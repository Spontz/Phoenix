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
		void		warmExec();
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
		if ((param.size()) != 2 || (shaderBlock.size() != 1)) {
			Logger::error("DrawQuad [{}]: 2 params (Clear the screen buffer & clear depth buffer), and 1 shader block are needed", identifier);
			return false;
		}

		render_disableDepthTest = true;
		// Load parameters
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);

		// Load shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename);
		if (!m_pShader)
			return false;

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (auto& uni : shaderBlock[0]->uniform) {
			m_pVars->ReadString(uni);
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues(type_str+"["+identifier+"]");
		
		return !DEMO_checkGLError();
	}

	void sDrawQuad::init()
	{

	}

	void sDrawQuad::warmExec()
	{
		exec();
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
		return debugStatic;
	}
}