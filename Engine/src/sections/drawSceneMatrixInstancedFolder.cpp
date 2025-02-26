#include "main.h"
#include "core/renderer/ModelInstance.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawSceneMatrixInstancedFolder final : public Section {
	public:
		sDrawSceneMatrixInstancedFolder();
		~sDrawSceneMatrixInstancedFolder();

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
		int32_t		m_iAnimationNumber = 0;	// Number of animation to play
		float		m_fAnimationTime = 0;	// Animation time (in seconds)
		int32_t		m_iNumInstancesPerObject = 1; // Number of instances to draw per each object
		float		m_fNumInstancesPerObject = 1.0f; // Number of instances to draw per each object (in float, for formulas)
		float		m_fNumTotalInstances = 0;	// Total number of object to draw

		// Matrix object positioning
		glm::vec3	m_vInsTranslation = { 0, 0, 0 };	// Matrix object translation
		glm::vec3	m_vInsRotation = { 0, 0, 0 };	// Matrix object rotation
		glm::vec3	m_vInsScale = { 1, 1, 1 };	// Matrix object scale

		// Current object positioning
		float		m_fCurrInsID = 0;			// Current object to draw
		glm::vec3	m_vCurrInsPos = { 0, 0, 0 };	// Current position of our object
		glm::vec3	m_vCurrInsPosPolar = { 0, 0, 0 };	// Current position of our object (in polar coordinates)
		glm::vec3	m_vCurrInsTranslation = { 0, 0, 0 };	// Current object translation
		glm::vec3	m_vCurrInsRotation = { 0, 0, 0 };	// Current object rotation
		glm::vec3	m_vCurrInsScale = { 1, 1, 1 };	// Current object scale


		// Previous model, projection and view matrix, for being used in effects like motion blur
		glm::mat4	m_mPrevProjection = glm::mat4(1.0f);
		glm::mat4	m_mPrevView = glm::mat4(1.0f);

		std::string						m_pFolder;				// Folder to scan
		std::vector<std::string>		m_pModelFilePaths;		// Models filePath to load
		std::vector<SP_Model>			m_pModel;				// Models to load
		std::vector<SP_ModelInstance>	m_pModelInstance;		// Instanced models to draw
		SP_Shader						m_pShader;
		MathDriver*						m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
		ShaderVars*						m_pVars = nullptr;			// For storing any other shader variables
	};

	// TODO:
	// 1- Pasar las coordenadas polares y cartesianas al shader (tiene sentido?)
	// 2- Pasar el "previous model matix al shader para poderlo usar con efectos como el motion blur

	// ******************************************************************

	Section* instance_drawSceneMatrixInstancedFolder()
	{
		return new sDrawSceneMatrixInstancedFolder();
	}

	sDrawSceneMatrixInstancedFolder::sDrawSceneMatrixInstancedFolder()
	{
		type = SectionType::DrawSceneMatrixInstancedFolder;
	}

	sDrawSceneMatrixInstancedFolder::~sDrawSceneMatrixInstancedFolder()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;

		//m_pModel.clear();
		//m_pModelInstance.clear();

		/*
		// Deltes all model instances
		for (auto const& pModelInstance : m_pModelInstance) {
			delete pModelInstance;
		}
		m_pModelInstance.clear();
		*/
		// TODO: FIX A BUG CLEARING DATA!!
	}

	bool sDrawSceneMatrixInstancedFolder::load()
	{
		if ((param.size() != 7) || (strings.size() != 2) || (shaderBlock.size() != 1)) {
			Logger::error(
				"DrawSceneMatrixInstancedFolder [{}]: 7 param (number of instances per model, do depth buffer clearing, "
				"disbale depth test, enable wireframe, update formulas on each frame, enable animation and animation "
				"number), 2 strings (models folder and extension), 1 shader and 1 expression are needed", identifier);
			return false;
		}

		Logger::info(LogLevel::high, "DrawSceneMatrixInstancedFolder [{}]: Warning! you are using an experimental section, probably will not behave correctly!!", identifier);


		// Instances per object
		m_iNumInstancesPerObject = static_cast<int32_t>(param[0]);
		m_fNumInstancesPerObject = param[0];
		if (m_iNumInstancesPerObject == 0) {
			Logger::error("DrawSceneMatrixInstancedFolder [{}]: Number of object instances cannot be 0", identifier);
			return false;
		}

		// Render Flags
		render_clearDepth = static_cast<bool>(param[1]);
		render_disableDepthTest = static_cast<bool>(param[2]);
		render_drawWireframe = static_cast<bool>(param[3]);

		// Update formulas on each frame
		m_bUpdateFormulas = static_cast<bool>(param[4]);

		// Animation parameters
		m_bPlayAnimation = static_cast<bool>(param[5]);
		m_iAnimationNumber = static_cast<int32_t>(param[6]);

		// Load model folder and the models contained in it
		m_pFolder = m_demo.m_dataFolder + strings[0];
		m_pModelFilePaths = Utils::getFilepathsFromFolder(m_pFolder, strings[1]);
		for (const auto& filePath : m_pModelFilePaths) {
			SP_Model model = m_demo.m_modelManager.addModel(filePath);
			if (model)
				m_pModel.emplace_back(model);
		}

		if (m_pModel.empty()) {
			Logger::error("DrawSceneMatrixInstancedFolder [{}]: No objects loaded", identifier);
			return false;
		}

		if (m_pModel.size() != m_pModelFilePaths.size()) {
			Logger::error("DrawSceneMatrixInstancedFolder [{}]: Not all objects loaded", identifier);
			return false;
		}

		// Load shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename);
		if (!m_pShader)
			return false;

		// Load instanced objects
		m_fNumTotalInstances = (float)m_iNumInstancesPerObject * (float)m_pModel.size(); // Total number of objects to draw

		/*
		for (int32_t i=0; i < m_pModel.size(); i++) {
			const auto pNewModelInstance = new ModelInstance(m_pModel[i], m_iNumInstancesPerObject);

			if (pNewModelInstance) {
				pNewModelInstance->m_pModel->playAnimation = m_bPlayAnimation;
				if (pNewModelInstance->m_pModel->playAnimation)
					pNewModelInstance->m_pModel->setAnimation(m_iAnimationNumber);

				m_pModelInstance.emplace_back(pNewModelInstance);
			}
		}
		*/
		for (int32_t i = 0; i < m_pModel.size(); i++) {
			SP_ModelInstance pNewModelInstance = std::make_shared<ModelInstance>(m_pModel[i], m_iNumInstancesPerObject);

			if (pNewModelInstance) {
				pNewModelInstance->m_pModel->playAnimation = m_bPlayAnimation;
				if (pNewModelInstance->m_pModel->playAnimation)
					pNewModelInstance->m_pModel->setAnimation(m_iAnimationNumber);

				m_pModelInstance.emplace_back(pNewModelInstance);
			}
		}

		m_pExprPosition = new MathDriver(this);
		m_pExprPosition->expression = expressionRun;

		m_pExprPosition->SymbolTable.add_variable("AnimationTime", m_fAnimationTime);
		m_pExprPosition->SymbolTable.add_variable("n", m_fCurrInsID);
		m_pExprPosition->SymbolTable.add_variable("instances", m_fNumInstancesPerObject);
		m_pExprPosition->SymbolTable.add_variable("n_total", m_fNumTotalInstances);
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
			Logger::error("DrawSceneMatrixInstancedFolder [{}]: Error while compiling the expression, default values used", identifier);

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (auto& uni : shaderBlock[0]->uniform) {
			m_pVars->ReadString(uni);
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues();

		// Update object matrices
		updateMatrices(true);

		return !DEMO_checkGLError();
	}

	void sDrawSceneMatrixInstancedFolder::init()
	{

	}

	void sDrawSceneMatrixInstancedFolder::warmExec()
	{
		exec();
	}

	void sDrawSceneMatrixInstancedFolder::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

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
		m_pExprPosition->executeFormula();

		// Update Matrices with objects positions, if required
		if (m_bUpdateFormulas)
			updateMatrices(false);

		m_pShader->setValue("n_total", m_fNumTotalInstances);	// Send total objects to draw to the shader

		// Draw Objects
		for (const auto& modelInstance : m_pModelInstance) {
			modelInstance->m_pModel->playAnimation = m_bPlayAnimation;
			if (modelInstance->m_pModel->playAnimation)
				modelInstance->m_pModel->setAnimation(m_iAnimationNumber);

			modelInstance->drawInstanced(m_pShader, m_fAnimationTime, static_cast<uint32_t>(m_pVars->sampler2D.size()));
		}

		// For MotionBlur: store the previous matrix
		m_mPrevProjection = projection;
		m_mPrevView = view;

		glUseProgram(0);

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawSceneMatrixInstancedFolder::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Expression is: " << (m_pExprPosition->isValid() ? "Valid" : "Faulty or Empty") << std::endl;
		ss << "Folder scanned: " << m_pFolder << std::endl;
		ss << "Objects found: " << m_pModel.size() << std::endl;
		ss << "Instances per object: " << m_iNumInstancesPerObject << std::endl;
		ss << "Total objects drawn: " << m_fNumTotalInstances << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawSceneMatrixInstancedFolder::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}

	void sDrawSceneMatrixInstancedFolder::updateMatrices(bool initPrevMatrix)
	{
		glm::mat4 matrixModel; // Model matrix to be used on matrix object

		m_fCurrInsID = 0;
		int object = 0;

		// Evaluate the expression
		m_pExprPosition->executeFormula();

		matrixModel = glm::mat4(1.0f);
		matrixModel = glm::translate(matrixModel, m_vInsTranslation);
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vInsRotation.x), glm::vec3(1, 0, 0));
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vInsRotation.y), glm::vec3(0, 1, 0));
		matrixModel = glm::rotate(matrixModel, glm::radians(m_vInsRotation.z), glm::vec3(0, 0, 1));
		matrixModel = glm::scale(matrixModel, m_vInsScale);


		glm::mat4* my_obj_model;

		for (int32_t i = 0; i < m_pModelInstance.size(); i++)
		{
			SP_ModelInstance mi = m_pModelInstance[i];

			for (int32_t j = 0; j < m_iNumInstancesPerObject; j++)
			{
				m_vCurrInsPos = glm::vec3(0); // All objects are located in pos 0 by default
				//m_vCurrObjPosPolar = glm::vec3(0); // All objects are located in pos 0 by default
				// Evaluate the expression
				m_pExprPosition->executeFormula();

				// Copy previous model object matrix, before model matrix changes
				// todo: move ouside the FOR??
				mi->copyMatrices(object);

				my_obj_model = &(mi->m_pModelMatrix[object]);
				*my_obj_model = matrixModel;
				*my_obj_model = glm::translate(*my_obj_model, m_vCurrInsPos);

				// Now render the object using the "model_ref" as a model matrix start position
				*my_obj_model = glm::translate(*my_obj_model, m_vCurrInsTranslation);
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.x), glm::vec3(1, 0, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.y), glm::vec3(0, 1, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.z), glm::vec3(0, 0, 1));
				*my_obj_model = glm::scale(*my_obj_model, m_vCurrInsScale);

				// In case we set this flag, the previous matrix is loaded again
				// This is required on the first call of this function, where the prev_model and obj_model matrix are not initilized
				// TODO: Move outside this for???
				if (initPrevMatrix)
					mi->copyMatrices(object);

				object++;
				m_fCurrInsID = static_cast<float>(object);
			}

			mi->updateMatrices(); // Update Matrices to GPU
		}
	}
}