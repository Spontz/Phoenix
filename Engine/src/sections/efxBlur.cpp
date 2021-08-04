#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"


namespace Phoenix {

	struct sEfxBlur : public Section {
	public:
		sEfxBlur();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		unsigned int	m_uiFboNum = 0;			// Fbo to use (must have 2 color attachments!)
		float			m_fBlurAmount = 1.0;	// Blur layers to apply
		Shader*			m_pShader = nullptr;	// Blur Shader to apply
		MathDriver*		m_pExprBlur = nullptr;	// Equations for the Blur effect
		ShaderVars*		m_pVars = nullptr;		// Shader variables
	};

	// ******************************************************************

	Section* instance_efxBlur()
	{
		return new sEfxBlur();
	}

	sEfxBlur::sEfxBlur()
	{
		type = SectionType::EfxBlur;
	}


	bool sEfxBlur::load()
	{
		// script validation
		if ((param.size()) != 3 || (strings.size() != 2)) {
			Logger::error("EfxBlur [%s]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 2 strings (One with the formula of the Blur Amount + blur shader file)", identifier.c_str());
			return false;
		}

		// Load parameters
		render_enableDepthTest = false;
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		m_uiFboNum = static_cast<unsigned int>(param[2]);

		// Check if the fbo can be used for the effect
		if (m_uiFboNum < 0 || m_uiFboNum >= m_demo.m_fboManager.fbo.size()) {
			Logger::error("EfxBlur [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", identifier.c_str(), m_uiFboNum, m_demo.m_fboManager.fbo.size() - 1);
			return false;
		}

		// Load the Blur amount formula
		m_pExprBlur = new MathDriver(this);
		// Load positions, process constants and compile expression
		m_pExprBlur->expression = strings[0]; // The first string should contain the blur amount
		m_pExprBlur->SymbolTable.add_variable("blurAmount", m_fBlurAmount);
		m_pExprBlur->Expression.register_symbol_table(m_pExprBlur->SymbolTable);
		if (!m_pExprBlur->compileFormula())
			return false;

		// Load Blur shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[1]);
		if (!m_pShader)
			return false;

		// Configure Blur shader
		m_pShader->use();
		m_pShader->setValue("image", 0);	// The image is in the texture unit 0

		m_pVars = new ShaderVars(this, m_pShader);
		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}
		// Set shader variables values
		m_pVars->setValues();

		return true;
	}

	void sEfxBlur::init()
	{

	}

	void sEfxBlur::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression
		m_pExprBlur->Expression.value();

		{
			// First step: Blur the image from the "fbo attachment 0", and store it in our efxBloom fbo manager (efxBloomFbo)
			bool horizontal = true;
			bool first_iteration = true;
			m_pShader->use();

			// Prevent negative Blurs
			if (m_fBlurAmount < 0)
				m_fBlurAmount = 0;
			unsigned int iBlurAmount = static_cast<unsigned int>(m_fBlurAmount);
			for (unsigned int i = 0; i < iBlurAmount; i++)
			{
				m_pShader->setValue("horizontal", horizontal);

				// We always draw the First pass in the efxBloom FBO
				m_demo.m_efxBloomFbo.bind(horizontal, false, false); // TODO: Fix: use an FBO for Blur, not the Bloom FBO

				// If it's the first iteration, we pick the fbo
				// if not, we pick the fbo of our efxBloom
				if (first_iteration)
					m_demo.m_fboManager.bind_tex(m_uiFboNum);
				else
					m_demo.m_efxBloomFbo.bind_tex(!horizontal);

				// Render scene
				m_demo.m_pRes->Draw_QuadFS();
				horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}

			m_demo.m_efxBloomFbo.unbind(false, false); // Unbind drawing into an Fbo

			// Adjust back the current fbo
			m_demo.m_fboManager.bindCurrent();
			// Second step: Draw the Blurred image
			m_demo.m_pRes->Draw_QuadEfxFBOFS(!horizontal);
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sEfxBlur::end()
	{

	}

	void sEfxBlur::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->m_filepath << std::endl;
		ss << "Fbo: " << m_uiFboNum << std::endl;
		debugStatic = ss.str();
	}

	std::string sEfxBlur::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "blurAmount: " << m_fBlurAmount << std::endl;
		return ss.str();
	}
}