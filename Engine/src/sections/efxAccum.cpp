#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sEfxAccum final : public Section
	{
	public:
		sEfxAccum();
		~sEfxAccum();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		float		m_lastTime = 0;
		bool		m_firstIteration = true;
		uint32_t	m_uiFboNum = 0;				// Fbo to use (must have 2 color attachments!)
		float		m_fSourceInfluence = .5f;	// Source influence (0 to 1)
		float		m_fAccumInfluence = .5f;	// Accumulation influence (0 to 1)
		bool		m_bAccumBuffer = false;		// Accum buffer to use (0 or 1)
		SP_Shader	m_spShader;					// Accumulation Shader to apply
		MathDriver* m_pExprAccum = nullptr;		// Equations for the Accum effect
		ShaderVars* m_pVars = nullptr;			// Shader variables
	};

	// ******************************************************************

	Section* instance_efxAccum()
	{
		return new sEfxAccum();
	}

	sEfxAccum::sEfxAccum()
	{
		type = SectionType::EfxAccum;
	}

	sEfxAccum::~sEfxAccum()
	{
		if (m_pExprAccum)
			delete m_pExprAccum;
		if (m_pVars)
			delete m_pVars;
	}

	bool sEfxAccum::load()
	{
		// script validation
		if ((param.size()) != 3 || (strings.size() < 1)) {
			Logger::error("EfxAccum [{}]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 1 string (accum shader)", identifier);
			return false;
		}

		// Load parameters
		render_disableDepthTest = true;
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		m_uiFboNum = static_cast<unsigned int>(param[2]);
		m_bAccumBuffer = false;

		// Check if the fbo can be used for the effect
		if (m_uiFboNum < 0 || m_uiFboNum >= m_demo.m_fboManager.fbo.size()) {
			Logger::error("EfxBlur [{}]: The fbo specified [{}] is not supported, should be between 0 and {}", identifier, m_uiFboNum, m_demo.m_fboManager.fbo.size() - 1);
			return false;
		}

		// Load the Blur amount formula
		m_pExprAccum = new MathDriver(this);
		// Load positions, process constants and compile expression
		for (int i = 1; i < strings.size(); i++)
			m_pExprAccum->expression += strings[i];
		m_pExprAccum->SymbolTable.add_variable("SourceInfluence", m_fSourceInfluence);
		m_pExprAccum->SymbolTable.add_variable("AccumInfluence", m_fAccumInfluence);
		m_pExprAccum->Expression.register_symbol_table(m_pExprAccum->SymbolTable);
		if (!m_pExprAccum->compileFormula())
			return false;

		// Load Blur shader
		m_spShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
		if (!m_spShader)
			return false;

		// Configure shader
		m_spShader->use();
		m_spShader->setValue("sourceImage", 0);	// The source image will be in the texture unit 0
		m_spShader->setValue("accumImage", 1);	// The accumulated image will be in the texture unit 1

		m_pVars = new ShaderVars(this, m_spShader);
		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues(identifier);
		
		return !DEMO_checkGLError();
	}

	void sEfxAccum::init()
	{
		m_demo.m_efxAccumFbo.clearFbosColor();
	}

	void sEfxAccum::warmExec()
	{
		exec();
	}

	void sEfxAccum::exec()
	{
		// Check if ne need to clear buffers first
		if (m_lastTime >= runTime)
			m_demo.m_efxAccumFbo.clearFbosColor();

		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();


		// Calculate deltaTime
		float deltaTime = runTime - m_lastTime;
		m_lastTime = runTime;

		// Evaluate the expression
		m_pExprAccum->Expression.value();

		{

			{
				// We want to capture the frame in the "Accum Fbo", so first we use the previous fbo for storing the entire image
				m_demo.m_efxAccumFbo.bind(m_bAccumBuffer, false, false);

				float fps = 1.0f / 60.0f;
				m_spShader->use();
				m_spShader->setValue("sourceInfluence", m_fSourceInfluence * (deltaTime / fps));
				m_spShader->setValue("accumInfluence", 1 - (m_fAccumInfluence * (deltaTime / fps)));
				m_pVars->setValues();

				// Set the screen fbo in texture unit 0
				m_demo.m_fboManager.bind_tex(m_uiFboNum, 0);

				// Set the accumulation fbo in texture unit 1
				if (m_firstIteration)
					m_firstIteration = false;
				m_bAccumBuffer = !m_bAccumBuffer;
				m_demo.m_efxAccumFbo.bind_tex(m_bAccumBuffer, 1);

				// Render a quad using the Accum shader (combining the 2 Images)
				m_demo.m_pRes->drawQuadFS();

				m_demo.m_efxAccumFbo.unbind(false, false); // Unbind drawing into the "Accum Fbo"

				// Adjust back the current fbo
				m_demo.m_fboManager.bindCurrent();
			}


			// Second step: Draw the accum buffer
			m_demo.m_pRes->m_spShdrQuadTex->use();
			m_demo.m_pRes->m_spShdrQuadTex->setValue("screenTexture", 0);
			if (m_firstIteration)
				m_demo.m_fboManager.bind_tex(m_uiFboNum, 0);
			else
				m_demo.m_efxAccumFbo.bind_tex(!m_bAccumBuffer, 0);
			m_demo.m_pRes->drawQuadFS();

		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sEfxAccum::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_spShader->getURI() << std::endl;
		ss << "Fbo: " << m_uiFboNum << std::endl;
		debugStatic = ss.str();
	}

	std::string sEfxAccum::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "Source Influence: " << m_fSourceInfluence << std::endl;
		ss << "Accum Influence: " << m_fAccumInfluence << std::endl;
		return ss.str();
	}
}