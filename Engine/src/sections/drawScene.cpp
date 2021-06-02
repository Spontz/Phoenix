#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	struct sDrawScene : public Section {
	public:
		sDrawScene();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		bool		m_bPlayAnimation = false;	// Do we want to play the animation?
		float		m_fCameraNumber = -1;		// Number of the camera to use (-1 = means to not use camera)
		int			m_iAnimationNumber = 0;		// Number of animation to play
		float		m_fAnimationTime = 0;		// Animation time (in seconds)

		glm::vec3	m_vTranslation = { 0, 0, 0 };
		glm::vec3	m_vRotation = { 0, 0, 0 };
		glm::vec3	m_vScale = { 1, 1, 1 };

		// Model, projection and view matrix
		glm::mat4	m_mModel = glm::mat4(1.0f);
		glm::mat4	m_mProjection = glm::mat4(1.0f);
		glm::mat4	m_mView = glm::mat4(1.0f);

		// Previous model, projection and view matrix, for being used in effects like motion blur
		glm::mat4	m_mPrevModel = glm::mat4(1.0f);
		glm::mat4	m_mPrevProjection = glm::mat4(1.0f);
		glm::mat4	m_mPrevView = glm::mat4(1.0f);

		Model* m_pModel = nullptr;
		Shader* m_pShader = nullptr;
		MathDriver* m_pExprPosition = nullptr;	// A equation containing the calculations to position the object
		ShaderVars* m_pVars = nullptr;	// For storing any other shader variables
	};

	// ******************************************************************

	Section* instance_drawScene() {
		return new sDrawScene();
	}

	sDrawScene::sDrawScene()
	{
		type = SectionType::DrawScene;
	}

	bool sDrawScene::load() {
		if ((this->param.size() != 6) || (this->strings.size() < 7)) {
			Logger::error(
				"DrawScene [%s]: 5 param (depth buffer clearing, enable depth test, enable depth mask, enable wireframe, enable animation & "
				"animation number) and 7 strings needed (model, shader, CameraNumber, aTime & "
				"three more for object positioning)",
				identifier.c_str());
			return false;
		}

		// Load parameters
		render_clearDepth = static_cast<bool>(param[0]);
		render_enableDepthTest = static_cast<bool>(param[1]);
		render_enableDepthMask = static_cast<bool>(param[2]);
		render_drawWireframe = static_cast<bool>(param[3]);
		
		m_bPlayAnimation = static_cast<bool>(param[4]);
		m_iAnimationNumber = static_cast<int>(param[5]);

		// Load model and shader
		m_pModel = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[0]);
		if (!m_pModel)
			return false;
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[1]);
		if (!m_pShader)
			return false;

		// Load model properties
		m_pModel->playAnimation = m_bPlayAnimation;
		if (m_pModel->playAnimation)
			m_pModel->setAnimation(m_iAnimationNumber);

		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (int i = 2; i < strings.size(); i++)
			m_pExprPosition->expression += strings[i];

		m_pExprPosition->SymbolTable.add_variable("CameraNumber", m_fCameraNumber);
		m_pExprPosition->SymbolTable.add_variable("aTime", m_fAnimationTime);
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
			return false;
		m_pExprPosition->Expression.value();
		// Set the camera number
		if (m_fCameraNumber < 0)
			m_pModel->useCamera = false;
		else
			m_pModel->setCamera((unsigned int)m_fCameraNumber);


		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}
		
		// Set shader variables values
		m_pVars->setValues();

		return true;
	}

	void sDrawScene::init()
	{

	}

	void sDrawScene::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		// Set model properties
		m_pModel->playAnimation = m_bPlayAnimation;
		if (m_pModel->playAnimation)
			m_pModel->setAnimation(m_iAnimationNumber);
		if (m_fCameraNumber < 0)
			m_pModel->useCamera = false;
		else
			m_pModel->setCamera((unsigned int)m_fCameraNumber);

		// Load shader
		m_pShader->use();

		// For ShadowMapping
		m_pShader->setValue("lightSpaceMatrix", m_demo.m_lightManager.light[0]->spaceMatrix);
		// End ShadowMapping

		// view/projection transformations
		m_mProjection = m_demo.m_pActiveCamera->getProjection();

		//m_pShader->setValue("projection", m_mProjection);
		m_pModel->m_matProjection = m_mProjection;

		m_mView = m_demo.m_pActiveCamera->getView();
		//if (CameraNumber < 0)
		//	m_pShader->setValue("view", m_mView);
		m_pModel->m_matView = m_mView;


		// render the loaded scene
		m_mModel = glm::mat4(1.0f);
		m_mModel = glm::translate(m_mModel, m_vTranslation);
		m_mModel = glm::rotate(m_mModel, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
		m_mModel = glm::rotate(m_mModel, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
		m_mModel = glm::rotate(m_mModel, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
		m_mModel = glm::scale(m_mModel, m_vScale);
		m_pModel->m_matBaseModel = m_mModel;

		// Set previousmatrices for being used in special effects like like MotionBlur
		//m_pShader->setValue("prev_projection", m_mPrevProjection);
		//if (CameraNumber < 0)
			//m_pShader->setValue("prev_view", m_mPrevView);
		//m_pShader->setValue("prev_model", m_mPrevModel);

		m_mPrevProjection = m_mProjection;
		//if (CameraNumber < 0)
		m_mPrevView = m_mView;
		//m_mPrevModel = m_mModel;
		// End MotionBlur


		// Set the other shader variable values
		m_pVars->setValues();

		m_pModel->Draw(m_pShader->ID, m_fAnimationTime, static_cast<uint32_t>(m_pVars->sampler2D.size()));

		glUseProgram(0);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawScene::end() {

	}

	void sDrawScene::loadDebugStatic() {
		std::stringstream ss;
		Material* mat;
		ss << "File: " << m_pModel->filename << std::endl;
		ss << "Meshes: " << m_pModel->m_statNumMeshes << ", Vertex: " << m_pModel->m_statNumVertices << std::endl;
		ss << "Animations: " << m_pModel->m_statNumAnimations << ", Bones: " << m_pModel->m_statNumBones << std::endl;
		ss << "Cameras: " << m_pModel->m_statNumCameras << std::endl;
		for (auto& mesh : m_pModel->meshes) {
			ss << "+Mesh name: " << mesh.m_nodeName << std::endl;
			ss << " Vertices: " << mesh.m_numVertices << std::endl;
			mat = mesh.getMaterial();
			ss << " Material name: " << mat->name << std::endl;
			ss << "  Material.Ka:" << glm::to_string(mat->colAmbient) << std::endl;
			ss << "  Material.Kd:" << glm::to_string(mat->colDiffuse) << std::endl;
			ss << "  Material.Ks:" << glm::to_string(mat->colSpecular) << std::endl;
			ss << "  Material.KsStrenght:" << std::to_string(mat->strenghtSpecular) << std::endl;
			for (auto& texture : mat->textures) {
				ss << "  Texture shader name: " << texture.shaderName << std::endl;
				ss << "  Texture file: " << texture.tex->filename << std::endl;
			}
			ss << std::endl;
		}
		debugStatic = ss.str();
	}


	std::string sDrawScene::debug()
	{
		return debugStatic;
	}
}