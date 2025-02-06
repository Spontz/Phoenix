#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawImage final : public Section {
	public:
		sDrawImage();
		~sDrawImage();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		bool		m_bFullscreen = true;		// Draw image at fullscreen?
		bool		m_bFitToContent = false;	// Fit to content: true:respect image aspect ratio, false:stretch to viewport/quad
		bool		m_bFilter = true;			// Use Bilinear filter?

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		float		m_fTexAspectRatio = 1.0f;
		float		m_fRenderAspectRatio = 1.0f;
		SP_Texture	m_pTexture;
		int32_t		m_iImageTexUnitID = 0;
		SP_Shader	m_pShader;
		MathDriver* m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars* m_pVars = nullptr;			// For storing any other shader variables
	};

	// ******************************************************************

	Section* instance_drawImage()
	{
		return new sDrawImage();
	}

	sDrawImage::sDrawImage()
	{
		type = SectionType::DrawImage;
	}

	sDrawImage::~sDrawImage()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
	}

	bool sDrawImage::load()
	{
		if ((param.size() != 5) || (strings.size() != 1) || (shaderBlock.size() != 1)) {
			Logger::error(
				"DrawImage [{}]: 5 param needed (Clear screen buffer, clear depth buffer, fullscreen, "
				"fit to content & filter), 1 string needed (Image), 1 shader and 1 expression",
				identifier
			);
			return false;
		}

		render_disableDepthTest = true;
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		m_bFullscreen = static_cast<bool>(param[2]);
		m_bFitToContent = static_cast<bool>(param[3]);
		m_bFilter = static_cast<bool>(param[4]);

		// Load the Image
		Texture::Properties texProps;
		texProps.m_useLinearFilter = m_bFilter;
		m_pTexture = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[0], texProps);
		if (!m_pTexture)
			return false;
		m_fTexAspectRatio = static_cast<float>(m_pTexture->m_width) / static_cast<float>(m_pTexture->m_height);

		// Load the Shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename);
		if (!m_pShader)
			return false;

		// Load the formmula containing the Image position and scale
		m_pExprPosition = new MathDriver(this);
		m_pExprPosition->expression = expressionRun;

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
		m_pExprPosition->SymbolTable.add_constant("texWidth", static_cast<float>(m_pTexture->m_width));
		m_pExprPosition->SymbolTable.add_constant("texHeight", static_cast<float>(m_pTexture->m_height));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			Logger::error("Draw Image [{}]: Error while compiling the expression, default values used", identifier);
		
		m_pExprPosition->executeFormula();

		// Create shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (auto& uni : shaderBlock[0]->uniform) {
			m_pVars->ReadString(uni);
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues(type_str + "[" + identifier + "]");
		
		// Set Image Texture unit ID, which will be the last of all the sampler2D that we have in all the shader variables
		m_iImageTexUnitID = static_cast<int32_t>(m_pVars->sampler2D.size());

		return !DEMO_checkGLError();
	}

	void sDrawImage::init()
	{
	}

	void sDrawImage::warmExec()
	{
		exec();
	}

	void sDrawImage::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression if we are not in fullscreen
		if (!m_bFullscreen)
			m_pExprPosition->executeFormula();

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
			m_pShader->setValue("screenTexture", m_iImageTexUnitID);
			m_pTexture->bind(m_iImageTexUnitID);
			
			// Set other shader variables values
			m_pVars->setValues();

			m_demo.m_pRes->drawQuadFS(); // Draw a quad with the video
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawImage::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "File: " << m_pTexture->m_filename << std::endl;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Expression is: " << (m_pExprPosition->isValid() ? "Valid" : "Faulty or Empty") << std::endl;
		ss << "Fullscreen: " << (m_bFullscreen ? "Yes":"No") << std::endl;
		ss << "Fit To Content: " << (m_bFitToContent ? "Yes":"No") << std::endl;
		ss << "Bilinear filter: " << (m_bFilter ? "Yes":"No") << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawImage::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "Pos: " << std::format("({:.2f},{:.2f},{:.2f})", m_vTranslation.x, m_vTranslation.y, m_vTranslation.z) << std::endl;
		ss << "Rot: " << std::format("({:.2f},{:.2f},{:.2f})", m_vRotation.x, m_vRotation.y, m_vRotation.z) << std::endl;
		ss << "Scale: " << std::format("({:.2f},{:.2f},{:.2f})", m_vScale.x, m_vScale.y, m_vScale.z) << std::endl;
		return ss.str();
	}
}