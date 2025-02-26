#include "main.h"
#include "core/renderer/Video.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawVideo final : public Section {
	public:
		sDrawVideo();
		~sDrawVideo();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		bool		m_bFullscreen = true;		// Draw video at fullscreen?
		bool		m_bFitToContent = false;	// Fit to content: true:respect video aspect ratio, false:stretch to viewport.

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		float		m_fVideoAspectRatio = 1.0f;
		float		m_fRenderAspectRatio = 1.0f;
		float		m_fvideoRunTime = 0.0f;
		float		m_fvideoDuration = 0.0f;
		SP_Video	m_pVideo;
		int32_t		m_iVideoTexUnitID = 0;
		SP_Shader	m_pShader;
		MathDriver*	m_pExprPosition = nullptr;	// A equation containing the calculations to position the object
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

	sDrawVideo::~sDrawVideo()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
	}

	bool sDrawVideo::load()
	{
		if ((param.size() != 4) || (strings.size() != 1) || (shaderBlock.size() != 1)) {
			Logger::error(
				"DrawVideo [{}]: 4 param needed (Clear screen buffer, clear depth buffer, fullscreen &"
				"fit to content), 1 string (Video), 1 shader and 1 expression are needed",
				identifier);
			return false;
		}

		render_disableDepthTest = true;
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		m_bFullscreen = static_cast<bool>(param[2]);
		m_bFitToContent = static_cast<bool>(param[3]);

		// Load the Video
		m_pVideo = m_demo.m_videoManager.addVideo(
			{
				m_demo.m_dataFolder + strings[0],
				-1, // hack: hardcoded, take from spo
				10, // hack: hardcoded, take from spo
				1.0 // hack: hardcoded, take from spo
			},
			m_demo.m_slaveMode
		);
		if (!m_pVideo)
			return false;
		m_fVideoAspectRatio = static_cast<float>(m_pVideo->getWidth()) / static_cast<float>(m_pVideo->getHeight());
		m_fvideoDuration = static_cast<float>(m_pVideo->videoDurationSecs());

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
		m_pExprPosition->SymbolTable.add_constant("vidWidth", static_cast<float>(m_pVideo->getWidth()));
		m_pExprPosition->SymbolTable.add_constant("vidHeight", static_cast<float>(m_pVideo->getHeight()));

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			Logger::error("DrawVideo [{}]: Error while compiling the expression, default values used", identifier);

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (auto& uni : shaderBlock[0]->uniform) {
			m_pVars->ReadString(uni);
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues();

		// Set Video Texture unit ID, which will be the last of all the sampler2D that we have in all the shader variables
		m_iVideoTexUnitID = static_cast<int32_t>(m_pVars->sampler2D.size());

		
		return !DEMO_checkGLError();
	}

	void sDrawVideo::init()
	{
	}

	void sDrawVideo::warmExec()
	{
		exec();
	}

	void sDrawVideo::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Render the video frame if needed
		m_fvideoRunTime = std::fmodf(runTime, m_fvideoDuration); // Recalculate video time
		m_pVideo->renderVideo(m_fvideoRunTime);

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
			m_pShader->setValue("screenTexture", m_iVideoTexUnitID);
			m_pVideo->bind(m_iVideoTexUnitID);

			// Set other shader variables values
			m_pVars->setValues();
			m_demo.m_pRes->drawQuadFS(); // Draw a quad with the video
		}

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawVideo::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Expression is: " << (m_pExprPosition->isValid() ? "Valid" : "Faulty or Empty") << std::endl;
		ss << "File: " << m_pVideo->getFileName() << std::endl;
		ss << "Fullscreen: " << m_bFullscreen << ", Fit to content: " << m_bFitToContent << std::endl;
		ss << "Video Duration: " << m_fvideoDuration << std::endl;
		debugStatic = ss.str();

	}

	std::string sDrawVideo::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		ss << "Video run time: " << m_fvideoRunTime << std::endl;
		return ss.str();
	}
}