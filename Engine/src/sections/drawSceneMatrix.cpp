#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawSceneMatrix final : public Section {
	public:
		sDrawSceneMatrix();
		~sDrawSceneMatrix();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		void		updateMatrices(bool initPrevMatrix);

		bool		m_bUpdateFormulas = true;		// Update positions on each frame?
		bool		m_bPlayAnimation = false;		// Do we want to play the animation?
		int			m_iAnimationNumber = 0;			// Number of animation to play
		float		m_fAnimationTime = 0;			// Animation time (in seconds)
		float		m_fNumInstances = 0;			// Total number of instances to draw

		// Matrix instance positioning
		glm::vec3	m_vInsTranslation = { 0, 0, 0 };
		glm::vec3	m_vInsRotation = { 0, 0, 0 };
		glm::vec3	m_vInsScale = { 1, 1, 1 };
		glm::mat4*	m_pInsMatrixModel = nullptr;	// Model matrix for each instance

		// Current instance positioning
		float		m_fCurrInsID = 0;			// Current instance to draw
		glm::vec3	m_vCurrInsPos = { 0, 0, 0 };	// Current position of our instance
		glm::vec3	m_vCurrInsPosPolar = { 0, 0, 0 };	// Current position of our instance (in polar coordinates)
		glm::vec3	m_vCurrInsTranslation = { 0, 0, 0 };	// Current instance translation
		glm::vec3	m_vCurrInsRotation = { 0, 0, 0 };	// Current instance rotation
		glm::vec3	m_vCurrInsScale = { 1, 1, 1 };	// Current instance scale


		// Previous model, projection and view matrix, for being used in effects like motion blur
		glm::mat4*	m_pmPrevModel = nullptr;			// The model needs to be stored on a vector because we need to store the previous model matrix of each instance
		glm::mat4	m_mPrevProjection = glm::mat4(1.0f);
		glm::mat4	m_mPrevView = glm::mat4(1.0f);

		SP_Model	m_pModelRef;	// Matrix model to be use to store positions
		SP_Model	m_pModel;		// Model to draw
		SP_Shader	m_pShader;
		MathDriver* m_pExprPosition = nullptr;	// An equation containing the calculations to position the instance
		ShaderVars* m_pVars = nullptr;			// For storing any other shader variables
	};

	// ******************************************************************

	Section* instance_drawSceneMatrix()
	{
		return new sDrawSceneMatrix();
	}

	sDrawSceneMatrix::sDrawSceneMatrix()
	{
		type = SectionType::DrawSceneMatrix;
	}

	sDrawSceneMatrix::~sDrawSceneMatrix()
	{
		// Delete matrices
		if (m_pmPrevModel)
			delete[] m_pmPrevModel;
		if (m_pInsMatrixModel)
			delete[] m_pInsMatrixModel;

		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
	}

	bool sDrawSceneMatrix::load()
	{
		if ((param.size() != 6) || (strings.size() < 7)) {
			Logger::error("DrawSceneMatrix [{}]: 6 param (do depth buffer clearing, disable depth test, enable wireframe, update formulas on each frame, enable animation and animation number) and 7 strings needed", identifier);
			return false;
		}

		// Load parameters
		render_clearDepth = static_cast<bool>(param[0]);
		render_disableDepthTest = static_cast<bool>(param[1]);
		render_drawWireframe = static_cast<bool>(param[2]);
		m_bUpdateFormulas = static_cast<bool>(param[3]);
		m_bPlayAnimation = static_cast<bool>(param[4]);
		m_iAnimationNumber = static_cast<int>(param[5]);

		// Load ref. model, model and shader
		m_pModelRef = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[0]);
		m_pModel = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[1]);
		if (!m_pModelRef || m_pModel < 0)
			return false;

		// Load unique vertices for the reference model (it can take a while)
		m_pModelRef->loadUniqueVertices();

		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[2]);
		if (!m_pShader)
			return false;

		// Calculate the number of matrices that we need to store
		uint32_t numInstances = 0;
		for (auto& meshRef : m_pModelRef->meshes) {
			numInstances += static_cast<uint32_t>(meshRef->m_uniqueVertices.size());
		}

		if (numInstances == 0) {
			Logger::error("DrawSceneMatrix: No vertex found in the reference model");
			return false;
		}

		m_fNumInstances = static_cast<float>(numInstances); // Number of instances to draw is the total amount of unique_vertices to draw
		// Instance Model Matrix
		m_pInsMatrixModel = new glm::mat4[numInstances];

		// Previous Instance model Matrix
		m_pmPrevModel = new glm::mat4[numInstances];

		// Load model properties
		m_pModel->playAnimation = m_bPlayAnimation;
		if (m_pModel->playAnimation)
			m_pModel->setAnimation(m_iAnimationNumber);


		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (int i = 3; i < strings.size(); i++)
			m_pExprPosition->expression += strings[i];

		m_pExprPosition->SymbolTable.add_variable("aTime", m_fAnimationTime);
		m_pExprPosition->SymbolTable.add_variable("n", m_fCurrInsID);
		m_pExprPosition->SymbolTable.add_variable("n_total", m_fNumInstances);
		m_pExprPosition->SymbolTable.add_variable("x", m_vCurrInsPos.x);
		m_pExprPosition->SymbolTable.add_variable("y", m_vCurrInsPos.y);
		m_pExprPosition->SymbolTable.add_variable("z", m_vCurrInsPos.z);
		m_pExprPosition->SymbolTable.add_variable("a", m_vCurrInsPosPolar.x);
		m_pExprPosition->SymbolTable.add_variable("b", m_vCurrInsPosPolar.y);
		m_pExprPosition->SymbolTable.add_variable("r", m_vCurrInsPosPolar.z);
		m_pExprPosition->SymbolTable.add_variable("tx", m_vCurrInsTranslation.x);
		m_pExprPosition->SymbolTable.add_variable("ty", m_vCurrInsTranslation.y);
		m_pExprPosition->SymbolTable.add_variable("tz", m_vCurrInsTranslation.z);
		m_pExprPosition->SymbolTable.add_variable("rx", m_vCurrInsRotation.x);
		m_pExprPosition->SymbolTable.add_variable("ry", m_vCurrInsRotation.y);
		m_pExprPosition->SymbolTable.add_variable("rz", m_vCurrInsRotation.z);
		m_pExprPosition->SymbolTable.add_variable("sx", m_vCurrInsScale.x);
		m_pExprPosition->SymbolTable.add_variable("sy", m_vCurrInsScale.y);
		m_pExprPosition->SymbolTable.add_variable("sz", m_vCurrInsScale.z);
		m_pExprPosition->SymbolTable.add_variable("m_tx", m_vInsTranslation.x);
		m_pExprPosition->SymbolTable.add_variable("m_ty", m_vInsTranslation.y);
		m_pExprPosition->SymbolTable.add_variable("m_tz", m_vInsTranslation.z);
		m_pExprPosition->SymbolTable.add_variable("m_rx", m_vInsRotation.x);
		m_pExprPosition->SymbolTable.add_variable("m_ry", m_vInsRotation.y);
		m_pExprPosition->SymbolTable.add_variable("m_rz", m_vInsRotation.z);
		m_pExprPosition->SymbolTable.add_variable("m_sx", m_vInsScale.x);
		m_pExprPosition->SymbolTable.add_variable("m_sy", m_vInsScale.y);
		m_pExprPosition->SymbolTable.add_variable("m_sz", m_vInsScale.z);

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			return false;

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}

		// Set shader variables
		m_pVars->setValues();

		// Update instance matrices
		updateMatrices(true);

		return !DEMO_checkGLError();
	}

	void sDrawSceneMatrix::init()
	{

	}

	void sDrawSceneMatrix::warmExec()
	{
		exec();
	}

	void sDrawSceneMatrix::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Set model properties
		m_pModel->playAnimation = m_bPlayAnimation;
		if (m_pModel->playAnimation)
			m_pModel->setAnimation(m_iAnimationNumber);

		// Load shader
		m_pShader->use();

		// For ShadowMapping
		m_pShader->setValue("lightSpaceMatrix", m_demo.m_lightManager.light[0]->spaceMatrix);
		// End ShadowMapping

		// view/projection transformations
		glm::mat4 projection = m_demo.m_cameraManager.getActiveProjection();
		glm::mat4 view = m_demo.m_cameraManager.getActiveView();
		m_pShader->setValue("projection", projection);	// TODO: Should we send the projection/view matrices here?
		m_pShader->setValue("view", view);
		m_pModel->m_matProjection = projection;			// TODO2: Or should we copy them into the model like we do here?
		m_pModel->m_matView = view;
		// For MotionBlur: send the previous matrix
		m_pShader->setValue("prev_projection", m_mPrevProjection);
		m_pShader->setValue("prev_view", m_mPrevView);

		// Set the other shader variable values
		m_pVars->setValues();

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		// Update Matrices with instance positions, if required
		if (m_bUpdateFormulas)
			updateMatrices(false);

		// Draw Instances
		int instance = 0;
		m_fCurrInsID = 0;
		m_pShader->setValue("n_total", m_fNumInstances);	// Send total instances to draw to the shader

		for (int i = 0; i < m_pModelRef->meshes.size(); i++)
		{
			for (int j = 0; j < m_pModelRef->meshes[i]->m_uniqueVertices.size(); j++)
			{
				m_vCurrInsPos = m_pModelRef->meshes[i]->m_uniqueVertices[j].Position;
				m_vCurrInsPosPolar = m_pModelRef->meshes[i]->m_uniqueVertices[j].PositionPolar;
				m_pShader->setValue("n", m_fCurrInsID);				// Send the number of instances to the shader
				m_pShader->setValue("n_pos", m_vCurrInsPos);			// Send the instance relative position to the shader
				m_pShader->setValue("n_polar", m_vCurrInsPosPolar);	// Send the instance relative position to the shader (in polar format: x=alpha, y=beta, z=distance)

				m_pModel->m_matBaseModel = m_pInsMatrixModel[instance];

				// For MotionBlur, we send the previous model matrix
				m_pShader->setValue("prev_model", m_pmPrevModel[instance]);

				m_pModel->Draw(m_pShader->getId(), m_fAnimationTime, static_cast<uint32_t>(m_pVars->sampler2D.size()));

				instance++;
				m_fCurrInsID = (float)instance;
			}
		}

		// For MotionBlur: store the previous matrix
		m_mPrevProjection = projection;
		m_mPrevView = view;

		glUseProgram(0);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawSceneMatrix::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Matrix file: " << m_pModelRef->filename << std::endl;
		ss << "Model instances drawn: " << m_fNumInstances << std::endl;
		ss << "Model file: " << m_pModel->filename << std::endl;
		ss << " Meshes in each model: " << m_pModel->m_statNumMeshes << std::endl;
		ss << " Model faces: " << m_pModel->m_statNumFaces << ", vertices: " << m_pModel->m_statNumVertices << std::endl;
		ss << "Total scene faces: " << m_pModel->m_statNumFaces * static_cast<uint32_t>(m_fNumInstances) << ", vertices: " << m_pModel->m_statNumVertices * static_cast<uint32_t>(m_fNumInstances) << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawSceneMatrix::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}

	void sDrawSceneMatrix::updateMatrices(bool initPrevMatrix)
	{
		glm::mat4 matModel; // Model matrix to be used on matrix instance

		m_fCurrInsID = 0;
		int instance = 0;

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, m_vInsTranslation);
		matModel = glm::rotate(matModel, glm::radians(m_vInsRotation.x), glm::vec3(1, 0, 0));
		matModel = glm::rotate(matModel, glm::radians(m_vInsRotation.y), glm::vec3(0, 1, 0));
		matModel = glm::rotate(matModel, glm::radians(m_vInsRotation.z), glm::vec3(0, 0, 1));
		matModel = glm::scale(matModel, m_vInsScale);

		glm::mat4* insMatModel;

		for (int i = 0; i < m_pModelRef->meshes.size(); i++)
		{
			for (int j = 0; j < m_pModelRef->meshes[i]->m_uniqueVertices.size(); j++)
			{
				m_vCurrInsPos = m_pModelRef->meshes[i]->m_uniqueVertices[j].Position;
				m_vCurrInsPosPolar = m_pModelRef->meshes[i]->m_uniqueVertices[j].PositionPolar;
				// Evaluate the expression
				m_pExprPosition->Expression.value();

				// Copy previous model instance matrix, before model matrix changes
				m_pmPrevModel[instance] = m_pInsMatrixModel[instance];

				insMatModel = &(m_pInsMatrixModel[instance]);
				*insMatModel = matModel;
				*insMatModel = glm::translate(*insMatModel, m_pModelRef->meshes[i]->m_uniqueVertices[j].Position);

				// Now render the instance using the "model_ref" as a model matrix start position
				*insMatModel = glm::translate(*insMatModel, m_vCurrInsTranslation);
				*insMatModel = glm::rotate(*insMatModel, glm::radians(m_vCurrInsRotation.x), glm::vec3(1, 0, 0));
				*insMatModel = glm::rotate(*insMatModel, glm::radians(m_vCurrInsRotation.y), glm::vec3(0, 1, 0));
				*insMatModel = glm::rotate(*insMatModel, glm::radians(m_vCurrInsRotation.z), glm::vec3(0, 0, 1));
				*insMatModel = glm::scale(*insMatModel, m_vCurrInsScale);

				// In case we set this flag, the previous matrix is loaded again
				// This is required on the first call of this function, where the prev_model and obj_model matrix are not initilized
				if (initPrevMatrix)
					m_pmPrevModel[instance] = m_pInsMatrixModel[instance];

				instance++;
				m_fCurrInsID = (float)instance;
			}
		}
	}
}