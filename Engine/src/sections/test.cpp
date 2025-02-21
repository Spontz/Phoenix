#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {
	class sTest final : public Section {
	public:
		sTest();
		~sTest();

	public:
		bool		load();
		void		init();
		void		exec();
		std::string debug();

	private:
		bool		m_bFullscreen = true;		// Draw image at fullscreen?
		bool		m_bFitToContent = false;	// Fit to content: true:respect image aspect ratio, false:stretch to viewport/quad
		bool		m_bFilter = true;			// Use Bilinear filter?

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		float						m_fTexAspectRatio = 1.0f;
		float						m_fRenderAspectRatio = 1.0f;
		SP_Texture					m_pTexture;
		std::string					m_pFolder;				// Folder to scan
		std::vector<std::string>	m_pModelFilePaths;		// Models filePath to load
		
		int32_t						m_iImageTexUnitID = 0;
		SP_Shader					m_pShader;
		MathDriver*					m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars*					m_pVars = nullptr;			// For storing any other shader variables

		glm::vec3					m_camPosition;
		glm::vec3					m_camDirection;
	};


	// ******************************************************************

	Section* instance_test() {
		return new sTest();
	}

	sTest::sTest() {
		type = SectionType::Test;
	}

	sTest::~sTest() {
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
	}

	bool sTest::load() {

		if ((param.size() != 5) || (strings.size() != 2) || (shaderBlock.size() != 1)) {
			Logger::error(
				"DrawImage3D [{}]: 5 param needed (Clear screen buffer, clear depth buffer, fullscreen, "
				"fit to content & filter), 2 string needed (Images folder and format), 1 shader and 1 expression",
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

		// Load the Images
		Texture::Properties texProps;
		texProps.m_useLinearFilter = m_bFilter;

		// Load texture folder and the textures contained in it
		m_pFolder = m_demo.m_dataFolder + strings[0];
		m_pModelFilePaths = Utils::getFilepathsFromFolder(m_pFolder, strings[1]);
		m_pTexture = m_demo.m_textureManager.addTexture(m_pModelFilePaths, texProps);
		if (!m_pTexture)
			return false;
		//m_fTexAspectRatio = static_cast<float>(m_pTexture->m_width) / static_cast<float>(m_pTexture->m_height);

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
			Logger::error("Draw Image 3D [{}]: Error while compiling the expression, default values used", identifier);

		m_pExprPosition->executeFormula();

		// Create shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (auto& uni : shaderBlock[0]->uniform) {
			m_pVars->ReadString(uni);
		}
		// Validate and set shader variables
		m_pVars->validateAndSetValues();

		// Set Image Texture unit ID, which will be the last of all the sampler2D that we have in all the shader variables
		m_iImageTexUnitID = static_cast<int32_t>(m_pVars->sampler2D.size());

		return !DEMO_checkGLError();
	}

	void sTest::init() {
	}

	void sTest::exec() {

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
			Camera *cam = m_demo.m_cameraManager.getActiveCamera();
			
			float yaw = glm::radians(cam->getYaw());
			float pitch = glm::radians(cam->getPitch());
			m_camPosition = cam->getPosition();

			m_camDirection.x = cos(yaw) * cos(pitch);
			m_camDirection.y = sin(pitch);
			m_camDirection.z = sin(yaw) * cos(pitch);
			m_camDirection = glm::normalize(m_camDirection);

			m_pShader->setValue("camDir", m_camDirection);
			mModel = glm::scale(mModel, glm::vec3(fXScale, fYScale, 0.0f));
			m_pShader->setValue("model", mModel);
			m_pShader->setValue("volume", m_iImageTexUnitID);
			m_pTexture->bind(m_iImageTexUnitID);

			// Set other shader variables values
			m_pVars->setValues();

			m_demo.m_pRes->drawCube(); // Draw a Cube with the volume inside
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();


	}

	std::string sTest::debug() {
		std::stringstream ss;
		ss << "Cam Pos: " << std::format("({:.2f},{:.2f},{:.2f})", m_camPosition.x, m_camPosition.y, m_camPosition.z) << std::endl;
		ss << "Cam Dir: " << std::format("({:.2f},{:.2f},{:.2f})", m_camDirection.x, m_camDirection.y, m_camDirection.z) << std::endl;
		return ss.str();
	}
}