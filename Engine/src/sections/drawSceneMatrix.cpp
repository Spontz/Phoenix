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
		bool		m_bPlayAnimation = false;	// Do we want to play the animation?
		int			m_iAnimationNumber = 0;		// Number of animation to play
		float		m_fAnimationTime = 0;		// Animation time (in seconds)
		float		m_fNumObjects = 0;		// Total number of object to draw

		// Matrix object positioning
		glm::vec3	m_vMatrixObjTranslation = { 0, 0, 0 };		// Matrix object translation
		glm::vec3	m_vMatrixObjRotation = { 0, 0, 0 };		// Matrix object rotation
		glm::vec3	m_vMatrixObjScale = { 1, 1, 1 };		// Matrix object scale
		glm::mat4*	m_pMatrixObjModel = nullptr;			// Model matrix for each object

		// Current object positioning
		float		m_fCurrObjID = 0;			// Current object to draw
		glm::vec3	m_vCurrObjPos = { 0, 0, 0 };	// Current position of our object
		glm::vec3	m_vCurrObjPosPolar = { 0, 0, 0 };	// Current position of our object (in polar coordinates)
		glm::vec3	m_vCurrObjTranslation = { 0, 0, 0 };	// Current object translation
		glm::vec3	m_vCurrObjRotation = { 0, 0, 0 };	// Current object rotation
		glm::vec3	m_vCurrObjScale = { 1, 1, 1 };	// Current object scale


		// Previous model, projection and view matrix, for being used in effects like motion blur
		glm::mat4*	m_pmPrevModel = nullptr;			// The model needs to be stored on a vector because we need to store the previous model matrix of each object
		glm::mat4	m_mPrevProjection = glm::mat4(1.0f);
		glm::mat4	m_mPrevView = glm::mat4(1.0f);

		SP_Model	m_pModelRef;	// Matrix model to be use to store positions
		SP_Model	m_pModel;		// Model to draw
		SP_Shader	m_pShader;
		MathDriver* m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
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
		if (m_pMatrixObjModel)
			delete[] m_pMatrixObjModel;

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
		uint32_t num_obj_instances = 0;
		for (auto& meshRef : m_pModelRef->meshes) {
			num_obj_instances += static_cast<uint32_t>(meshRef->m_uniqueVertices.size());
		}

		if (num_obj_instances == 0) {
			Logger::error("DrawSceneMatrix: No vertex found in the reference model");
			return false;
		}

		m_fNumObjects = static_cast<float>(num_obj_instances); // Number of objects to draw is the total amount of unique_vertices to draw
		// Object Model Matrix
		m_pMatrixObjModel = new glm::mat4[num_obj_instances];

		// Previous Object model Matrix
		m_pmPrevModel = new glm::mat4[num_obj_instances];

		// Load model properties
		m_pModel->playAnimation = m_bPlayAnimation;
		if (m_pModel->playAnimation)
			m_pModel->setAnimation(m_iAnimationNumber);


		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (int i = 3; i < strings.size(); i++)
			m_pExprPosition->expression += strings[i];

		m_pExprPosition->SymbolTable.add_variable("aTime", m_fAnimationTime);
		m_pExprPosition->SymbolTable.add_variable("n", m_fCurrObjID);
		m_pExprPosition->SymbolTable.add_variable("n_total", m_fNumObjects);
		m_pExprPosition->SymbolTable.add_variable("x", m_vCurrObjPos.x);
		m_pExprPosition->SymbolTable.add_variable("y", m_vCurrObjPos.y);
		m_pExprPosition->SymbolTable.add_variable("z", m_vCurrObjPos.z);
		m_pExprPosition->SymbolTable.add_variable("a", m_vCurrObjPosPolar.x);
		m_pExprPosition->SymbolTable.add_variable("b", m_vCurrObjPosPolar.y);
		m_pExprPosition->SymbolTable.add_variable("r", m_vCurrObjPosPolar.z);
		m_pExprPosition->SymbolTable.add_variable("tx", m_vCurrObjTranslation.x);
		m_pExprPosition->SymbolTable.add_variable("ty", m_vCurrObjTranslation.y);
		m_pExprPosition->SymbolTable.add_variable("tz", m_vCurrObjTranslation.z);
		m_pExprPosition->SymbolTable.add_variable("rx", m_vCurrObjRotation.x);
		m_pExprPosition->SymbolTable.add_variable("ry", m_vCurrObjRotation.y);
		m_pExprPosition->SymbolTable.add_variable("rz", m_vCurrObjRotation.z);
		m_pExprPosition->SymbolTable.add_variable("sx", m_vCurrObjScale.x);
		m_pExprPosition->SymbolTable.add_variable("sy", m_vCurrObjScale.y);
		m_pExprPosition->SymbolTable.add_variable("sz", m_vCurrObjScale.z);
		m_pExprPosition->SymbolTable.add_variable("m_tx", m_vMatrixObjTranslation.x);
		m_pExprPosition->SymbolTable.add_variable("m_ty", m_vMatrixObjTranslation.y);
		m_pExprPosition->SymbolTable.add_variable("m_tz", m_vMatrixObjTranslation.z);
		m_pExprPosition->SymbolTable.add_variable("m_rx", m_vMatrixObjRotation.x);
		m_pExprPosition->SymbolTable.add_variable("m_ry", m_vMatrixObjRotation.y);
		m_pExprPosition->SymbolTable.add_variable("m_rz", m_vMatrixObjRotation.z);
		m_pExprPosition->SymbolTable.add_variable("m_sx", m_vMatrixObjScale.x);
		m_pExprPosition->SymbolTable.add_variable("m_sy", m_vMatrixObjScale.y);
		m_pExprPosition->SymbolTable.add_variable("m_sz", m_vMatrixObjScale.z);

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

		// Update object matrices
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

		// Update Matrices with objects positions, if required
		if (m_bUpdateFormulas)
			updateMatrices(false);

		// Draw Objects
		int object = 0;
		m_fCurrObjID = 0;
		m_pShader->setValue("n_total", m_fNumObjects);	// Send total objects to draw to the shader

		for (int i = 0; i < m_pModelRef->meshes.size(); i++)
		{
			for (int j = 0; j < m_pModelRef->meshes[i]->m_uniqueVertices.size(); j++)
			{
				m_vCurrObjPos = m_pModelRef->meshes[i]->m_uniqueVertices[j].Position;
				m_vCurrObjPosPolar = m_pModelRef->meshes[i]->m_uniqueVertices[j].PositionPolar;
				m_pShader->setValue("n", m_fCurrObjID);				// Send the number of object to the shader
				m_pShader->setValue("n_pos", m_vCurrObjPos);			// Send the object relative position to the shader
				m_pShader->setValue("n_polar", m_vCurrObjPosPolar);	// Send the object relative position to the shader (in polar format: x=alpha, y=beta, z=distance)

				m_pModel->m_matBaseModel = m_pMatrixObjModel[object];

				// For MotionBlur, we send the previous model matrix
				m_pShader->setValue("prev_model", m_pmPrevModel[object]);

				m_pModel->Draw(m_pShader->getId(), m_fAnimationTime, static_cast<uint32_t>(m_pVars->sampler2D.size()));

				object++;
				m_fCurrObjID = (float)object;
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
		ss << "Objects in matrix to be drawn: " << m_fNumObjects << std::endl;
		ss << "Model file: " << m_pModel->filename << std::endl;
		ss << "Meshes in each model: " << m_pModel->m_statNumMeshes << std::endl;
		ss << "Faces: " << m_pModel->m_statNumFaces << ", vertices: " << m_pModel->m_statNumVertices << std::endl;
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
		glm::mat4 matrixModel; // Model matrix to be used on matrix object

		m_fCurrObjID = 0;
		int object = 0;

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		matrixModel = glm::mat4(1.0f);
		matrixModel = glm::translate(matrixModel, m_vMatrixObjTranslation);
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vMatrixObjRotation.x), glm::vec3(1, 0, 0));
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vMatrixObjRotation.y), glm::vec3(0, 1, 0));
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vMatrixObjRotation.z), glm::vec3(0, 0, 1));
		matrixModel = glm::scale(matrixModel, m_vMatrixObjScale);

		glm::mat4* my_obj_model;

		for (int i = 0; i < m_pModelRef->meshes.size(); i++)
		{
			for (int j = 0; j < m_pModelRef->meshes[i]->m_uniqueVertices.size(); j++)
			{
				m_vCurrObjPos = m_pModelRef->meshes[i]->m_uniqueVertices[j].Position;
				m_vCurrObjPosPolar = m_pModelRef->meshes[i]->m_uniqueVertices[j].PositionPolar;
				// Evaluate the expression
				m_pExprPosition->Expression.value();

				// Copy previous model object matrix, before model matrix changes
				m_pmPrevModel[object] = m_pMatrixObjModel[object];

				my_obj_model = &(m_pMatrixObjModel[object]);
				*my_obj_model = matrixModel;
				*my_obj_model = glm::translate(*my_obj_model, m_pModelRef->meshes[i]->m_uniqueVertices[j].Position);

				// Now render the object using the "model_ref" as a model matrix start position
				*my_obj_model = glm::translate(*my_obj_model, m_vCurrObjTranslation);
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrObjRotation.x), glm::vec3(1, 0, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrObjRotation.y), glm::vec3(0, 1, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrObjRotation.z), glm::vec3(0, 0, 1));
				*my_obj_model = glm::scale(*my_obj_model, m_vCurrObjScale);

				// In case we set this flag, the previous matrix is loaded again
				// This is required on the first call of this function, where the prev_model and obj_model matrix are not initilized
				if (initPrevMatrix)
					m_pmPrevModel[object] = m_pMatrixObjModel[object];

				object++;
				m_fCurrObjID = (float)object;
			}
		}
	}
}