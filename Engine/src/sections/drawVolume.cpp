#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {
	class sDrawVolume final : public Section {
	public:
		sDrawVolume();
		~sDrawVolume();

	public:
		bool		load();
		void		init();
		void		exec();
		std::string debug();

	private:
		int			m_volumeIndex = -1;			// Volume index to draw

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		// Model, projection and view matrix
		glm::mat4	m_mModel = glm::mat4(1.0f);
		glm::mat4	m_mInvModel = glm::mat4(1.0f);
		glm::mat4	m_mProjection = glm::mat4(1.0f);
		glm::mat4	m_mView = glm::mat4(1.0f);

		SP_Shader					m_pShader;
		MathDriver*					m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars*					m_pVars = nullptr;			// For storing any other shader variables
	};


	// ******************************************************************

	Section* instance_drawVolume() {
		return new sDrawVolume();
	}

	sDrawVolume::sDrawVolume() {
		type = SectionType::DrawVolume;
	}

	sDrawVolume::~sDrawVolume() {
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
	}

	bool sDrawVolume::load() {

		
		// 5 params = current/legacy drawVolume
		// 6 params = current/legacy params + runtime 3D texture index
		if ((param.size() != 4 && param.size() != 5) || (shaderBlock.size() != 1)) {
			Logger::error( "Draw Volume [{}]: 4 or 5 params needed " "(depth buffer clearing, disable depth test, disable depth mask & "
				"enable wireframe [, runtime 3D texture index]), " "1 shader and 1 expression", identifier );
			return false;
		}

		// Load parameters
		render_clearDepth = static_cast<bool>(param[0]);
		render_disableDepthTest = static_cast<bool>(param[1]);
		render_disableDepthMask = static_cast<bool>(param[2]);
		render_drawWireframe = static_cast<bool>(param[3]);
		
		// Optional runtime 3D texture
		// param[4] contains the logical index into
		// TextureManager::runtime3D.
		// -1 means: do not use a runtime 3D texture
		m_volumeIndex = -1;
		if (param.size() == 5) {
			m_volumeIndex = static_cast<int>(param[4]);
			if (m_volumeIndex < 0) {
				Logger::error( "Draw Volume [{}]: invalid runtime 3D texture index {}", identifier, m_volumeIndex );
				return false;
			}
			const auto volume = m_demo.m_textureManager.getRuntime3D( m_volumeIndex );
			if (!volume) {
				Logger::error( "Draw Volume [{}]: runtime 3D texture {} not found", identifier, m_volumeIndex );
				return false;
			}
		}

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

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			Logger::error("Draw Volume [{}]: Error while compiling the expression, default values used", identifier);

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

		// If this drawVolume uses a runtime 3D texture, bind the
		// sampler to texture unit 0.
		// The actual texture binding is performed every frame in exec().
		// Here we only configure the sampler uniform.
		if (m_volumeIndex >= 0) {
			if (m_pShader->getUniformLocation("volume") == -1) {
				Logger::error( "Draw Volume [{}]: runtime 3D texture requested, " "but shader '{}' does not contain the 'volume' sampler3D uniform", identifier, m_pShader->getURI() );
				return false;
			}
			m_pShader->setValue("volume", 0);
		}

		return !DEMO_checkGLError();
	}

	void sDrawVolume::init() {
	}

	void sDrawVolume::exec() {

		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();
		{
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
			m_mInvModel = glm::inverse(m_mModel);

			// Send uniform variables to the shader
			m_pShader->setValue("projection", m_mProjection);
			m_pShader->setValue("view", m_mView);
			m_pShader->setValue("model", m_mModel);
			m_pShader->setValue("invModel", m_mInvModel);

			// If we use runtime 3D texture, bind it to texture unit 0 and set the sampler uniform
			if (m_volumeIndex >= 0) {
				auto volume = m_demo.m_textureManager.getRuntime3D(m_volumeIndex);
				if (!volume)
					return;
				volume->bind(0);
				m_pShader->setValue("volume", 0);
				// TODO: What happens if in the shader we use another Sampler3D uniform? Should we bind the texture to another unit and set the uniform accordingly? For now, we assume that the shader uses the 'volume' uniform for the runtime 3D texture.
			}

			// Set the other shader uniform variable values
			m_pVars->setValues();

			m_demo.m_pRes->drawCube(); // Draw a Cube with the volume inside

			glUseProgram(0);
		}
		
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	std::string sDrawVolume::debug() {
		std::stringstream ss;
		return ss.str();
	}
}