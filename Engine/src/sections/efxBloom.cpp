#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sEfxBloom final : public Section {
	public:
		sEfxBloom();
		~sEfxBloom();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		unsigned int	m_uiFboNum = 0;			// Fbo to use (must have 2 color attachments!)
		float			m_fBlurAmount = 1;		// Blur layers to apply
		SP_Shader		m_pShaderBlur;			// Blur Shader to apply
		SP_Shader		m_pShaderBloom;			// Bloom Shader to apply
		MathDriver*		m_pExprBloom = nullptr;	// Equations for the Bloom effect
		ShaderVars*		m_pVars = nullptr;		// Shader variables
	};

	// ******************************************************************

	Section* instance_efxBloom()
	{
		return new sEfxBloom();
	}

	sEfxBloom::sEfxBloom()
	{
		type = SectionType::EfxBloom;
	}

	sEfxBloom::~sEfxBloom()
	{
		if (m_pExprBloom)
			delete m_pExprBloom;
		if (m_pVars)
			delete m_pVars;
	}

	bool sEfxBloom::load()
	{
		// script validation
		if ((param.size()) != 3 || (strings.size() != 3)) {
			Logger::error("EfxBloom [{}]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 3 strings (One with the formula of the Blur Amount + 2 with the Blur and Bloom shaders)", identifier);
			return false;
		}

		// Load parameters
		render_disableDepthTest = true;
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		m_uiFboNum = static_cast<unsigned int>(param[2]);

		// Check if the fbo can be used for the effect
		if (m_uiFboNum < 0 || m_uiFboNum >= m_demo.m_fboManager.fbo.size()) {
			Logger::error("EfxBloom [{}]: The fbo specified [{}] is not supported, should be between 0 and {}", identifier, m_uiFboNum, m_demo.m_fboManager.fbo.size() - 1);
			return false;
		}
		if (m_demo.m_fboManager.fbo[m_uiFboNum]->numAttachments < 2) {
			Logger::error("EfxBloom [{}]: The fbo specified [{}] has less than 2 attachments, so it cannot be used for bloom effect: Attahment 0 is the color image and Attachment 1 is the brights image", identifier, m_uiFboNum);
			return false;
		}

		// Load the Blur amount formula
		m_pExprBloom = new MathDriver(this);
		// Load positions, process constants and compile expression
		m_pExprBloom->expression = strings[0]; // The first string should contain the blur amount
		m_pExprBloom->SymbolTable.add_variable("blurAmount", m_fBlurAmount);
		m_pExprBloom->Expression.register_symbol_table(m_pExprBloom->SymbolTable);
		if (!m_pExprBloom->compileFormula())
			return false;

		// Load Blur shader
		m_pShaderBlur = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[1]);
		// Load Bloom shader
		m_pShaderBloom = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[2]);
		if (!m_pShaderBlur || !m_pShaderBloom)
			return false;

		// Create Shader variables

		// Configure Blur shader
		m_pShaderBlur->use();
		m_pShaderBlur->setValue("image", 0);	// The image is in the texture unit 0

		// Configure Bloom shader (variables are for this shader)
		m_pShaderBloom->use();
		m_pVars = new ShaderVars(this, m_pShaderBloom);
		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}
		// Validate ans set shader variables
		m_pVars->validateAndSetValues();
		m_pShaderBloom->setValue("scene", 0);		// The scene is in the Tex unit 0
		m_pShaderBloom->setValue("bloomBlur", 1);	// The bloom blur is in the Tex unit 1
		
		return !DEMO_checkGLError();
	}

	void sEfxBloom::init()
	{

	}

	void sEfxBloom::warmExec()
	{
		exec();
	}

	void sEfxBloom::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression
		m_pExprBloom->Expression.value();

		{
			// First step: Blur the image from the "fbo attachment 1", and store it in our efxBloom fbo manager (efxBloomFbo)
			bool horizontal = true;
			bool first_iteration = true;
			m_pShaderBlur->use();

			// Prevent negative Blurs
			if (m_fBlurAmount < 0)
				m_fBlurAmount = 0;
			unsigned int iBlurAmount = static_cast<unsigned int>(m_fBlurAmount);
			for (unsigned int i = 0; i < iBlurAmount; i++)
			{
				m_pShaderBlur->setValue("horizontal", horizontal);

				// We always draw the First pass in the efxBloom FBO
				m_demo.m_efxBloomFbo.bind(horizontal, false, false);

				// If it's the first iteration, we pick the second attachment of our fbo
				// if not, we pick the fbo of our efxBloom
				if (first_iteration)
					m_demo.m_fboManager.bind_tex(m_uiFboNum, 0, 1); //Use the second attachment of the fbo
				else
					m_demo.m_efxBloomFbo.bind_tex(!horizontal, 0, 0);	// Use the texture from our efxBloom

				// Render scene
				m_demo.m_pRes->drawQuadFS();
				horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}
			m_demo.m_efxBloomFbo.unbind(false, false); // Unbind drawing into an Fbo

			// Second step: Merge the blurred image with the color image (fbo attachment 0)
			m_pShaderBloom->use();
			// Set new shader variables values
			m_pVars->setValues();

			// Tex unit 0: scene
			m_demo.m_fboManager.fbo[m_uiFboNum]->bind_tex(0);
			// Tex unit 1: Bloom blur
			auto tmp = m_demo.m_efxBloomFbo.fbo;
			m_demo.m_efxBloomFbo.fbo[!horizontal]->bind_tex(1);

			// Adjust back the current fbo
			m_demo.m_fboManager.bindCurrent();
			m_demo.m_pRes->drawQuadFS();
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sEfxBloom::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader Bloom: " << m_pShaderBloom->getURI() << std::endl;
		ss << "Shader Blur: " << m_pShaderBlur->getURI() << std::endl;
		ss << "Fbo: " << m_uiFboNum << std::endl;
		debugStatic = ss.str();
	}

	std::string sEfxBloom::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "blurAmount: " << m_fBlurAmount << std::endl;
		return ss.str();
	}
}