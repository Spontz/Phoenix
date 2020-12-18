#include "main.h"
#include "core/renderer/Video.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawVideo final : public Section {
	public:
		sDrawVideo();

	public:
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		bool		m_bClearScreen = false;	// Clear Screen buffer
		bool		m_bClearDepth = false;	// Clear Depth buffer
		bool		m_bFullscreen = true;		// Draw video at fullscreen?
		bool		m_bFitToContent = false;	// Fit to content: true:respect video aspect ratio, false:stretch to viewport.

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		float		m_fVideoAspectRatio = 1.0f;
		float		m_fRenderAspectRatio = 1.0f;
		Video* m_pVideo = nullptr;
		Shader* m_pShader = nullptr;
		MathDriver* m_pExprPosition = nullptr;	// A equation containing the calculations to position the object
		ShaderVars* m_pVars = nullptr;	// Shader variables
	};

	// ******************************************************************

	Section* instance_drawVideo()
	{
		return new sDrawVideo();
	}

	sDrawVideo::sDrawVideo()
	{
		type = SectionType::DrawVideo;
	}

	bool sDrawVideo::load()
	{
		if ((param.size() != 4) || (strings.size() < 5)) {
			Logger::error(
				"DrawVideo [%s]: 4 param needed (Clear screen buffer, clear depth buffer, fullscreen &"
				"fit to content) and 5 strings needed (Video & shader paths and 3 for positon)",
				identifier.c_str());
			return false;
		}

		m_bClearScreen = static_cast<bool>(param[0]);
		m_bClearDepth = static_cast<bool>(param[1]);
		m_bFullscreen = static_cast<bool>(param[2]);
		m_bFitToContent = static_cast<bool>(param[3]);

		// Load the Video
		m_pVideo = m_demo.m_videoManager.addVideo(
			{
				m_demo.m_dataFolder + strings[0],
				-1, // hack: hardcoded, take from spo
				10, // hack: hardcoded, take from spo
				1.0 // hack: hardcoded, take from spo
			}
		);
		if (!m_pVideo)
			return false;
		m_fVideoAspectRatio = static_cast<float>(m_pVideo->getWidth()) / static_cast<float>(m_pVideo->getHeight());

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
		m_pExprPosition->SymbolTable.add_constant("vidWidth", static_cast<float>(m_pVideo->getWidth()));
		m_pExprPosition->SymbolTable.add_constant("vidHeight", static_cast<float>(m_pVideo->getHeight()));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (std::string const& s : uniform)
			m_pVars->ReadString(s.c_str());

		// Set shader variables values
		m_pVars->setValues();

		return true;
	}

	void sDrawVideo::init()
	{

	}

	void sDrawVideo::exec()
	{
		m_pVideo->renderVideo(runTime);

		if (m_bClearScreen)
			glClear(GL_COLOR_BUFFER_BIT);

		if (m_bClearDepth)
			glClear(GL_DEPTH_BUFFER_BIT);

		// Evaluate the expression if we are not in fullscreen
		if (!m_bFullscreen)
			m_pExprPosition->Expression.value();

		EvalBlendingStart();
		glDisable(GL_DEPTH_TEST);
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
				if (m_fVideoAspectRatio > m_fRenderAspectRatio)
					fYScale = m_fRenderAspectRatio / m_fVideoAspectRatio;
				else
					fXScale = m_fVideoAspectRatio / m_fRenderAspectRatio;
			}

			fYScale = -fYScale;	// Invert Y

			// Calculate Matrix depending if we are on fullscreen or not
			if (m_bFullscreen)
			{
				m_pShader->setValue("projection", glm::identity<glm::mat4>());
				m_pShader->setValue("view", glm::identity<glm::mat4>());
			}
			else
			{
				glm::mat4 mView = m_demo.m_pCamera->GetViewMatrix();
				float zoom = m_demo.m_pCamera->Zoom;
				glm::mat4 mProjection = glm::perspective(glm::radians(zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);

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
			m_pVideo->bind(0); // todo: from SPO maybe?
			m_demo.m_pRes->Draw_QuadFS(); // Draw a quad with the video

		}
		glEnable(GL_DEPTH_TEST);

		EvalBlendingEnd();
	}

	void sDrawVideo::end()
	{
	}

	void sDrawVideo::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "File: " << m_pVideo->getFileName() << std::endl;
		ss << "Fullscreen: " << m_bFullscreen << ", Fit to content: " << m_bFitToContent << std::endl;
		debugStatic = ss.str();

	}

	std::string sDrawVideo::debug()
	{
		return debugStatic;
	}
}