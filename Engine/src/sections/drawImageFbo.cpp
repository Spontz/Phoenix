#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"


// TODO: Pintar una FBO pudiendo elegir el shader y posicionandola en la pantalla como hacemos con el drawImage

namespace Phoenix {

	class sDrawImageFbo final : public Section {
	public:
		sDrawImageFbo();
		~sDrawImageFbo();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		// Fbo to draw
		Fbo* m_pFbo = nullptr;
		uint32_t		m_uFboNum = 0;
		uint32_t		m_uFboAttachment = 0;

		bool		m_bFullscreen = true;		// Draw image at fullscreen?
		bool		m_bFitToContent = false;	// Fit to content: true:respect image aspect ratio, false:stretch to viewport/quad
		bool		m_bFilter = true;			// Use Bilinear filter?

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		float		m_fTexAspectRatio = 1.0f;
		float		m_fRenderAspectRatio = 1.0f;
		SP_Shader	m_pShader;
		MathDriver* m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars* m_pVars = nullptr;			// For storing any other shader variables
	};

	// ******************************************************************

	Section* instance_drawImageFbo()
	{
		return new sDrawImageFbo();
	}

	sDrawImageFbo::sDrawImageFbo()
	{
		type = SectionType::DrawImageFbo;
	}

	sDrawImageFbo::~sDrawImageFbo()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
	}

	bool sDrawImageFbo::load()
	{
		if ((param.size() != 7) || (strings.size() < 4)) {
			Logger::error(
				"DrawImageFbo [{}]: 7 param needed (Fbo Number, Fbo Attachment, Clear screen buffer, clear depth buffer, fullscreen, "
				"fit to content & filter) and 4 strings needed (shader and 3 for position)",
				identifier
			);
			return false;
		}

		render_disableDepthTest = true;

		// Load Fbo number
		m_uFboNum = static_cast<uint32_t>(param[0]);
		m_uFboAttachment = static_cast<uint32_t>(param[1]);


		render_clearColor = static_cast<bool>(param[2]);
		render_clearDepth = static_cast<bool>(param[3]);
		m_bFullscreen = static_cast<bool>(param[4]);
		m_bFitToContent = static_cast<bool>(param[5]);
		m_bFilter = static_cast<bool>(param[6]);

		// Check for the right parameter values
		if (m_uFboNum >= FBO_BUFFERS) {
			Logger::error("Draw Image Fbo [{}]: Invalid fbo number: {}", identifier, m_uFboNum);
			return false;
		}

		m_pFbo = m_demo.m_fboManager.fbo[m_uFboNum];

		m_fTexAspectRatio = static_cast<float>(m_pFbo->width) / static_cast<float>(m_pFbo->height);

		// Load the Shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
		if (!m_pShader)
			return false;

		// Load the formmula containing the Image position and scale
		m_pExprPosition = new MathDriver(this);
		// Load positions, process constants and compile expression
		for (size_t i = 1; i < strings.size(); i++)
			m_pExprPosition->expression += strings[i];
		m_pExprPosition->SymbolTable.add_variable("tx", m_vTranslation.x);
		m_pExprPosition->SymbolTable.add_variable("ty", m_vTranslation.y);
		m_pExprPosition->SymbolTable.add_variable("tz", m_vTranslation.z);
		m_pExprPosition->SymbolTable.add_variable("rx", m_vRotation.x);
		m_pExprPosition->SymbolTable.add_variable("ry", m_vRotation.y);
		m_pExprPosition->SymbolTable.add_variable("rz", m_vRotation.z);
		m_pExprPosition->SymbolTable.add_variable("sx", m_vScale.x);
		m_pExprPosition->SymbolTable.add_variable("sy", m_vScale.y);
		m_pExprPosition->SymbolTable.add_variable("sz", m_vScale.z);
		// Add constants
		m_pExprPosition->SymbolTable.add_constant("fboWidth", static_cast<float>(m_pFbo->width));
		m_pExprPosition->SymbolTable.add_constant("fboHeight", static_cast<float>(m_pFbo->height));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Create shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (std::string const& s : uniform)
			m_pVars->ReadString(s.c_str());

		// Set shader variables
		m_pVars->setValues();
		
		return !DEMO_checkGLError();
	}

	void sDrawImageFbo::init()
	{
	}

	void sDrawImageFbo::warmExec()
	{
		exec();
	}

	void sDrawImageFbo::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression if we are not in fullscreen
		if (!m_bFullscreen)
			m_pExprPosition->Expression.value();

		{
			m_pShader->use();
			glm::mat4 mModel = glm::identity<glm::mat4>();

			// Render aspect ratio, stored for Keeping image proportions
			if (m_bFullscreen)
				m_fRenderAspectRatio = m_demo.m_Window->GetCurrentViewport().GetAspectRatio();
			else
				m_fRenderAspectRatio = m_vScale.x / m_vScale.y;

			// Calculate Scale factors
			float fXScale = 1;
			float fYScale = 1;
			if (m_bFitToContent) {
				if (m_fTexAspectRatio > m_fRenderAspectRatio)
					fYScale = m_fRenderAspectRatio / m_fTexAspectRatio;
				else
					fXScale = m_fTexAspectRatio / m_fRenderAspectRatio;
			}

			// Calculate Matrix depending if we are on fullscreen or not
			if (m_bFullscreen)
			{
				m_pShader->setValue("projection", glm::identity<glm::mat4>());
				m_pShader->setValue("view", glm::identity<glm::mat4>());
			}
			else
			{
				glm::mat4 mView = m_demo.m_cameraManager.getActiveView();
				glm::mat4 mProjection = m_demo.m_cameraManager.getActiveProjection();

				mModel = glm::translate(mModel, m_vTranslation);
				mModel = glm::rotate(mModel, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
				mModel = glm::rotate(mModel, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
				mModel = glm::rotate(mModel, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
				// Calc the new scale factors
				fXScale *= m_vScale.x * m_fRenderAspectRatio;
				fYScale *= m_vScale.y * m_fRenderAspectRatio;
				m_pShader->setValue("projection", mProjection);
				m_pShader->setValue("view", mView);
			}

			mModel = glm::scale(mModel, glm::vec3(fXScale, fYScale, 0.0f));
			m_pShader->setValue("model", mModel);
			m_pShader->setValue("screenTexture", 0);
			// Set other shader variables values
			m_pVars->setValues();
			// Set the FBO texture ID
			m_pFbo->bind_tex(0, m_uFboAttachment);

			m_demo.m_pRes->drawQuadFS(); // Draw a quad with the video
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawImageFbo::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Fbo num: " << m_uFboNum << std::endl;
		ss << "Fbo attachment: " << m_uFboAttachment << std::endl;
		ss << "Width: " << m_pFbo->width << std::endl;
		ss << "Height: " << m_pFbo->height << std::endl;
		ss << "Fullscreen: " << (m_bFullscreen ? "Yes":"No") << std::endl;
		ss << "Fit To Content: " << (m_bFitToContent ? "Yes":"No") << std::endl;
		ss << "Bilinear filter: " << (m_bFilter ? "Yes":"No") << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawImageFbo::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "Pos: " << glm::to_string(m_vTranslation) << std::endl;
		ss << "Rot: " << glm::to_string(m_vRotation) << std::endl;
		ss << "Scale: " << glm::to_string(m_vScale) << std::endl;
		return ss.str();
	}
}