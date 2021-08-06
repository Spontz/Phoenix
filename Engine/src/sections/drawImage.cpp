#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sDrawImage : public Section {
	public:
		sDrawImage();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		bool		m_bFullscreen = true;		// Draw image at fullscreen?
		bool		m_bFitToContent = false;	// Fit to content: true:respect image aspect ratio, false:stretch to viewport/quad


		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		float		m_fTexAspectRatio = 1.0f;
		float		m_fRenderAspectRatio = 1.0f;
		Texture* m_pTexture = nullptr;
		Shader* m_pShader = nullptr;
		MathDriver* m_pExprPosition = nullptr;	// A equation containing the calculations to position the object
		ShaderVars* m_pVars = nullptr;	// For storing any other shader variables
	} drawImage_section;

	// ******************************************************************

	Section* instance_drawImage()
	{
		return new sDrawImage();
	}

	sDrawImage::sDrawImage()
	{
		type = SectionType::DrawImage;
	}


	bool sDrawImage::load()
	{
		if ((param.size() != 4) || (strings.size() < 5)) {
			Logger::error(
				"DrawImage [%s]: 4 param needed (Clear screen buffer, clear depth buffer, fullscreen &"
				"fit to content) and 5 strings needed (Image & shader paths and 3 for position)",
				identifier.c_str());
			return false;
		}

		render_disableDepthTest = true;
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		m_bFullscreen = static_cast<bool>(param[2]);
		m_bFitToContent = static_cast<bool>(param[3]);

		// Load the Image
		m_pTexture = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + strings[0]);
		if (!m_pTexture)
			return false;
		m_fTexAspectRatio = static_cast<float>(m_pTexture->width) / static_cast<float>(m_pTexture->height);

		// Load the Shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[1]);
		if (!m_pShader)
			return false;

		// Load the formmula containing the Image position and scale
		m_pExprPosition = new MathDriver(this);
		// Load positions, process constants and compile expression
		for (int i = 2; i < strings.size(); i++)
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
		m_pExprPosition->SymbolTable.add_constant("texWidth", static_cast<float>(m_pTexture->width));
		m_pExprPosition->SymbolTable.add_constant("texHeight", static_cast<float>(m_pTexture->height));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Create shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (std::string const& s : uniform)
			m_pVars->ReadString(s.c_str());

		// Set shader variables values
		m_pVars->setValues();

		return true;
	}

	void sDrawImage::init()
	{
	}

	void sDrawImage::exec()
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
				m_fRenderAspectRatio = GLDRV->GetCurrentViewport().GetAspectRatio();
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
				glm::mat4 mView = m_demo.m_pActiveCamera->getView();
				glm::mat4 mProjection = m_demo.m_pActiveCamera->getProjection();

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
			m_pTexture->bind();
			m_demo.m_pRes->Draw_QuadFS(); // Draw a quad with the video

			/*
			// View / projection / model Matrixes
			glm::mat4 view = m_demo.camera->getViewMatrix();
			glm::mat4 projection = m_demo.m_pCamera->getProjectionMatrix();

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, m_vTranslation);
			model = glm::rotate(model, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
			model = glm::rotate(model, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
			model = glm::rotate(model, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
			model = glm::scale(model, glm::vec3(m_vScale.x, m_vScale.y*m_fTexAspectRatio, m_vScale.z));

			// Draw the image
			m_pShader->use();
			m_pShader->setValue("model", model);
			m_pShader->setValue("projection", projection);
			m_pShader->setValue("view", view);
			m_pShader->setValue("screenTexture", 0);

			m_pTexture->bind();

			// Set the values
			m_pVars->setValues();

			m_demo.res->Draw_QuadFS();
			*/
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawImage::end()
	{

	}

	void sDrawImage::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->m_filepath << std::endl;
		ss << "File: " << m_pTexture->filename << std::endl;
		ss << "Fullscreen: " << m_bFullscreen << std::endl;
		ss << "Fit To Content: " << m_bFitToContent << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawImage::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "Pos: " << glm::to_string(m_vTranslation) << std::endl;
		ss << "Rot: " << glm::to_string(m_vRotation) << std::endl;
		ss << "Scale: " << glm::to_string(m_vScale) << std::endl;
		return ss.str();
	}
}