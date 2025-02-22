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
		bool		m_bFilter = true;			// Use Bilinear filter?

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		// Model, projection and view matrix
		glm::mat4	m_mModel = glm::mat4(1.0f);
		glm::mat4	m_mProjection = glm::mat4(1.0f);
		glm::mat4	m_mView = glm::mat4(1.0f);

		SP_Texture					m_pTexture;
		std::string					m_pFolder;				// Folder to scan
		std::vector<std::string>	m_pModelFilePaths;		// Models filePath to load
		
		int32_t						m_iImageTexUnitID = 0;
		SP_Shader					m_pShader;
		MathDriver*					m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars*					m_pVars = nullptr;			// For storing any other shader variables
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
				"DrawImage3D [{}]: 5 param needed (depth buffer clearing, disable depth test, disable depth mask, enable wireframe & filter), "
				"2 string needed (Images folder and format), 1 shader and 1 expression",
				identifier
			);
			return false;
		}

		// Load parameters
		render_clearDepth = static_cast<bool>(param[0]);
		render_disableDepthTest = static_cast<bool>(param[1]);
		render_disableDepthMask = static_cast<bool>(param[2]);
		render_drawWireframe = static_cast<bool>(param[3]);
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
		{
			//glEnable(GL_CULL_FACE); glCullFace(GL_BACK);

			// Evaluate the expression
			m_pExprPosition->executeFormula();

			// Load shader
			m_pShader->use();

			// Calculate Projection, View and Model matrix
			m_mProjection = m_demo.m_cameraManager.getActiveProjection();
			m_mView = m_demo.m_cameraManager.getActiveView();

			m_mModel = glm::mat4(1.0f);
			m_mModel = glm::translate(m_mModel, m_vTranslation);
			m_mModel = glm::rotate(m_mModel, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
			m_mModel = glm::rotate(m_mModel, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
			m_mModel = glm::rotate(m_mModel, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
			m_mModel = glm::scale(m_mModel, m_vScale);

			// Send uniform variables to the shader
			m_pShader->setValue("projection", m_mProjection);
			m_pShader->setValue("view", m_mView);
			m_pShader->setValue("model", m_mModel);

			// Set the 3D volume as well
			m_pShader->setValue("uVolumeTex", m_iImageTexUnitID);
			m_pTexture->bind(m_iImageTexUnitID);

			// Set the other shader uniform variable values
			m_pVars->setValues();

			m_demo.m_pRes->drawCube(); // Draw a Cube with the volume inside

			glUseProgram(0);
			//glDisable(GL_CULL_FACE); glCullFace(GL_BACK);
		}
		
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();


	}

	std::string sTest::debug() {
		std::stringstream ss;
		return ss.str();
	}
}