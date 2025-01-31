#include "main.h"
#include "core/renderer/ModelInstance.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawSceneMatrixInstanced final : public Section {
	public:
		sDrawSceneMatrixInstanced();
		~sDrawSceneMatrixInstanced();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		void		updateMatrices(bool initPrevMatrix);

		bool		m_bUpdateFormulas = true;	// Update positions on each frame?
		bool		m_bPlayAnimation = false;// Do we want to play the animation?
		int			m_iAnimationNumber = 0;	// Number of animation to play
		float		m_fAnimationTime = 0;	// Animation time (in seconds)
		float		m_fNumInstances = 0;	// Total number of instances to draw

		// Matrix instance positioning
		glm::vec3	m_vInsTranslation = { 0, 0, 0 };
		glm::vec3	m_vInsRotation = { 0, 0, 0 };
		glm::vec3	m_vInsScale = { 1, 1, 1 };

		// Current instance positioning
		float		m_fCurrInsID = 0;			// Current instance to draw
		glm::vec3	m_vCurrInsPos = { 0, 0, 0 };
		glm::vec3	m_vCurrInsPosPolar = { 0, 0, 0 };
		glm::vec3	m_vCurrInsTranslation = { 0, 0, 0 };
		glm::vec3	m_vCurrInsRotation = { 0, 0, 0 };
		glm::vec3	m_vCurrInsScale = { 1, 1, 1 };


		// Previous model, projection and view matrix, for being used in effects like motion blur
		glm::mat4	m_mPrevProjection = glm::mat4(1.0f);
		glm::mat4	m_mPrevView = glm::mat4(1.0f);

		SP_Model		m_pModelRef;		// Reference model to be use to store positions
		ModelInstance*	m_pModel = nullptr;	// Model to draw instanced
		SP_Shader		m_pShader;
		MathDriver*		m_pExprPosition = nullptr;	// An equation containing the calculations to position the instance
		ShaderVars*		m_pVars = nullptr;			// For storing any other shader variables
	};

	// TODO:
	// 1- Pasar las coordenadas polares y cartesianas al shader (tiene sentido?)
	// 2- Pasar el "previous model matix al shader para poderlo usar con efectos como el motion blur

	// ******************************************************************

	Section* instance_drawSceneMatrixInstanced()
	{
		return new sDrawSceneMatrixInstanced();
	}

	sDrawSceneMatrixInstanced::sDrawSceneMatrixInstanced()
	{
		type = SectionType::DrawSceneMatrixInstanced;
	}

	sDrawSceneMatrixInstanced::~sDrawSceneMatrixInstanced()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
		if (m_pModel)
			delete m_pModel;	// TODO: Cambiar a Shared pointer??
	}

	bool sDrawSceneMatrixInstanced::load()
	{
		if ((param.size() != 6) || (strings.size() < 7)) {
			Logger::error("DrawSceneMatrixInstanced [{}]: 6 param (do depth buffer clearing, disbale depth test, enable wireframe, update formulas on each frame, enable animation and animation number) and 7 strings needed", identifier);
			return false;
		}

		Logger::info(LogLevel::high, "[sDrawSceneMatrixInstanced] Warning! you are using an experimental section, probably will not behave correctly!!");

		// Render Flags
		render_clearDepth = static_cast<bool>(param[0]);
		render_disableDepthTest = static_cast<bool>(param[1]);
		render_drawWireframe = static_cast<bool>(param[2]);

		// Update formulas on each frame
		m_bUpdateFormulas = static_cast<bool>(param[3]);

		// Animation parameters
		m_bPlayAnimation = static_cast<bool>(param[4]);
		m_iAnimationNumber = static_cast<int>(param[5]);

		// Load ref. model, model and shader
		m_pModelRef = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[0]);
		auto model_to_draw = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[1]);
		if (!m_pModelRef || !model_to_draw)
			return false;

		// Load unique vertices for the reference model (it can take a while)
		m_pModelRef->loadUniqueVertices();

		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[2]);
		if (!m_pShader)
			return false;

		// Calculate the amount of instances to draw
		uint32_t numInstances = 0;
		for (auto& meshRef : m_pModelRef->meshes) {
			numInstances += static_cast<uint32_t>(meshRef->m_uniqueVertices.size());
		}

		if (numInstances == 0) {
			Logger::error("DrawSceneMatrixInstanced: No vertex found in the reference model");
			return false;
		}

		m_fNumInstances = (float)numInstances; // Number of instances to draw is the total amount of unique_vertices to draw

		// Load model properties
		m_pModel = new ModelInstance(model_to_draw, numInstances);
		m_pModel->m_pModel->playAnimation = m_bPlayAnimation;
		if (m_pModel->m_pModel->playAnimation)
			m_pModel->m_pModel->setAnimation(m_iAnimationNumber);


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

		// Validate and set shader variables
		m_pVars->validateAndSetValues(type_str+"["+identifier+"]");

		// Update instance matrices
		updateMatrices(true);

		return !DEMO_checkGLError();
	}

	void sDrawSceneMatrixInstanced::init()
	{

	}

	void sDrawSceneMatrixInstanced::warmExec()
	{
		exec();
	}

	void sDrawSceneMatrixInstanced::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Set model properties
		m_pModel->m_pModel->playAnimation = m_bPlayAnimation;
		if (m_pModel->m_pModel->playAnimation)
			m_pModel->m_pModel->setAnimation(m_iAnimationNumber);

		// Load shader
		m_pShader->use();

		// For ShadowMapping
		m_pShader->setValue("lightSpaceMatrix", m_demo.m_lightManager.light[0]->spaceMatrix);
		// End ShadowMapping

		// view/projection transformations
		glm::mat4 projection = m_demo.m_cameraManager.getActiveProjection();
		glm::mat4 view = m_demo.m_cameraManager.getActiveView();
		m_pShader->setValue("projection", projection);
		m_pShader->setValue("view", view);
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

		// Draw instances
		//int instance = 0;
		m_fCurrInsID = 0;
		m_pShader->setValue("n_total", m_fNumInstances);	// Send total instances to draw to the shader

		// TODO: We should send the Position of each instance (in Cartesian ad Polar coordinates) to the shader, as we do in "drawSceneMatrix"
		// Does it makes sense to do this? we never used...
		/*
		for (int i = 0; i < model_ref->meshes.size(); i++)
		{
			for (int j = 0; j < model_ref->meshes[i].unique_vertices_pos.size(); j++)
			{
				m_cObjPos = model_ref->meshes[i].unique_vertices_pos[j];
				m_cObjPosPolar = model_ref->meshes[i].unique_vertices_polar[j];
				// Evaluate the expression
				exprPosition->Expression.value();
				shader->setValue("n", m_cObjID);				// Send the number of instance to the shader
				shader->setValue("n_pos", m_cObjPos);			// Send the instance relative position to the shader
				shader->setValue("n_polar", m_cObjPosPolar);	// Send the instance relative position to the shader (in polar format: x=alpha, y=beta, z=distance)

				instance++;
				m_cObjID = (float)instance;
			}
		}
		*/
		m_pModel->drawInstanced(m_pShader, m_fAnimationTime, static_cast<uint32_t>(m_pVars->sampler2D.size()));

		// For MotionBlur: store the previous matrix
		m_mPrevProjection = projection;
		m_mPrevView = view;

		glUseProgram(0);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawSceneMatrixInstanced::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Matrix file: " << m_pModelRef->filename << std::endl;
		ss << "Model instances drawn: " << m_fNumInstances << std::endl;
		ss << "Model file: " << m_pModel->m_pModel->filename << std::endl;
		ss << " Meshes in each model: " << m_pModel->m_pModel->m_statNumMeshes << std::endl;
		ss << " Model faces: " << m_pModel->m_pModel->m_statNumFaces << ", vertices: " << m_pModel->m_pModel->m_statNumVertices << std::endl;
		ss << "Total scene faces: " << m_pModel->m_stats.numTotalFaces << ", vertices: " << m_pModel->m_stats.numTotalVertices << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawSceneMatrixInstanced::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}

	void sDrawSceneMatrixInstanced::updateMatrices(bool initPrevMatrix)
	{
		glm::mat4 matrixModel; // Model matrix to be used on matrix instance

		m_fCurrInsID = 0;
		int instance = 0;

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		matrixModel = glm::mat4(1.0f);
		matrixModel = glm::translate(matrixModel, m_vInsTranslation);
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vInsRotation.x), glm::vec3(1, 0, 0));
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vInsRotation.y), glm::vec3(0, 1, 0));
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vInsRotation.z), glm::vec3(0, 0, 1));
		matrixModel = glm::scale(matrixModel, m_vInsScale);


		glm::mat4* my_obj_model;

		for (int i = 0; i < m_pModelRef->meshes.size(); i++)
		{
			for (int j = 0; j < m_pModelRef->meshes[i]->m_uniqueVertices.size(); j++)
			{
				m_vCurrInsPos = m_pModelRef->meshes[i]->m_uniqueVertices[j].Position;
				m_vCurrInsPosPolar = m_pModelRef->meshes[i]->m_uniqueVertices[j].PositionPolar;
				// Evaluate the expression
				m_pExprPosition->Expression.value();

				// Copy previous model instance matrix, before model matrix changes
				m_pModel->copyMatrices(instance);

				my_obj_model = &(m_pModel->m_pModelMatrix[instance]);
				*my_obj_model = matrixModel;
				*my_obj_model = glm::translate(*my_obj_model, m_pModelRef->meshes[i]->m_uniqueVertices[j].Position);

				// Now render the instance using the "model_ref" as a model matrix start position
				*my_obj_model = glm::translate(*my_obj_model, m_vCurrInsTranslation);
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.x), glm::vec3(1, 0, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.y), glm::vec3(0, 1, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.z), glm::vec3(0, 0, 1));
				*my_obj_model = glm::scale(*my_obj_model, m_vCurrInsScale);

				// In case we set this flag, the previous matrix is loaded again
				// This is required on the first call of this function, where the prev_model and obj_model matrix are not initilized
				if (initPrevMatrix)
					m_pModel->copyMatrices(instance);

				instance++;
				m_fCurrInsID = (float)instance;
			}
		}

		m_pModel->updateMatrices(); // Update Matrices to GPU
	}
}