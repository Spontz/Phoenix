#include "main.h"
#include "core/renderer/ModelInstance.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {

	class sDrawSceneMatrixFolder final : public Section {
	public:
		sDrawSceneMatrixFolder();
		~sDrawSceneMatrixFolder();

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
		int32_t		m_iAnimationNumber = 0;			// Number of animation to play
		float		m_fAnimationTime = 0;			// Animation time (in seconds)
		int32_t		m_iNumInstancesPerModel = 1;	// Number of instances to draw per each model
		float		m_fNumInstancesPerModel = 1.0f;	// Number of instances to draw per each model (in float, for formulas)
		float		m_fNumTotalInstances = 0;		// Total number of instances to draw

		// Matrix instance positioning
		glm::vec3	m_vInsTranslation = { 0, 0, 0 };
		glm::vec3	m_vInsRotation = { 0, 0, 0 };
		glm::vec3	m_vInsScale = { 1, 1, 1 };
		glm::mat4*	m_pInsMatrixModel = nullptr;

		// Current instance positioning
		float		m_fCurrInsID = 0;						// Current instance to draw
		glm::vec3	m_vCurrInsPos = { 0, 0, 0 };			// Current position of our instance
		glm::vec3	m_vCurrInsPosPolar = { 0, 0, 0 };		// Current position of our instance (in polar coordinates)
		glm::vec3	m_vCurrInsTranslation = { 0, 0, 0 };	// Current instance translation
		glm::vec3	m_vCurrInsRotation = { 0, 0, 0 };		// Current instance rotation
		glm::vec3	m_vCurrInsScale = { 1, 1, 1 };			// Current instance scale


		// Previous model, projection and view matrix, for being used in effects like motion blur
		glm::mat4*	m_pmPrevModel = nullptr;			// The model needs to be stored on a vector because we need to store the previous model matrix of each instance
		glm::mat4	m_mPrevProjection = glm::mat4(1.0f);
		glm::mat4	m_mPrevView = glm::mat4(1.0f);

		std::string						m_pFolder;				// Folder to scan
		std::vector<std::string>		m_pModelFilePaths;		// Models filePath to load
		std::vector<SP_Model>			m_pModel;				// Models to load
		SP_Shader						m_pShader;
		MathDriver*						m_pExprPosition = nullptr;	// An equation containing the calculations to position the instance
		ShaderVars*						m_pVars = nullptr;			// For storing any other shader variables
	};

	// TODO:
	// 1- Pasar las coordenadas polares y cartesianas al shader (tiene sentido?)
	// 2- Pasar el "previous model matix al shader para poderlo usar con efectos como el motion blur

	// ******************************************************************

	Section* instance_drawSceneMatrixFolder()
	{
		return new sDrawSceneMatrixFolder();
	}

	sDrawSceneMatrixFolder::sDrawSceneMatrixFolder()
	{
		type = SectionType::DrawSceneMatrixFolder;
	}

	sDrawSceneMatrixFolder::~sDrawSceneMatrixFolder()
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

	bool sDrawSceneMatrixFolder::load()
	{
		if ((param.size() != 7) || (strings.size() < 7)) {
			Logger::error("DrawSceneMatrixFolder [{}]: 7 param (number of instances per model, do depth buffer clearing, disbale depth test, enable wireframe, update formulas on each frame, enable animation and animation number) and 6 strings needed", identifier);
			return false;
		}

		// Copies per Model
		m_iNumInstancesPerModel = static_cast<int32_t>(param[0]);
		m_fNumInstancesPerModel = param[0];
		if (m_iNumInstancesPerModel == 0) {
			Logger::error("DrawSceneMatrixFolder: Number of instance copies cannot be 0");
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
			Logger::error("DrawSceneMatrixFolder: No instances loaded");
			return false;
		}

		if (m_pModel.size() != m_pModelFilePaths.size()) {
			Logger::error("DrawSceneMatrixFolder: Not all instances loaded!");
			return false;
		}

		// Load shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[2]);
		if (!m_pShader)
			return false;

		// Calculate the number of matrices that we need to store
		uint32_t numTotalInstances = m_iNumInstancesPerModel * static_cast<int32_t>(m_pModel.size());
		// Instance Model Matrix
		m_pInsMatrixModel = new glm::mat4[numTotalInstances];
		// Previous Instance model Matrix
		m_pmPrevModel = new glm::mat4[numTotalInstances];
		
		m_fNumTotalInstances = static_cast<float>(numTotalInstances);

		// Load model properties
		for (const auto& model : m_pModel) {
			model->playAnimation = m_bPlayAnimation;
			if (model->playAnimation)
				model->setAnimation(m_iAnimationNumber);
		}

		m_pExprPosition = new MathDriver(this);
		// Load all the other strings
		for (int i = 3; i < strings.size(); i++)
			m_pExprPosition->expression += strings[i];

		m_pExprPosition->SymbolTable.add_variable("aTime", m_fAnimationTime);
		m_pExprPosition->SymbolTable.add_variable("n", m_fCurrInsID);
		m_pExprPosition->SymbolTable.add_variable("copies", m_fNumInstancesPerModel);
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
			return false;

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (int i = 0; i < uniform.size(); i++) {
			m_pVars->ReadString(uniform[i].c_str());
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues(identifier);

		// Update instance matrices
		updateMatrices(true);

		return !DEMO_checkGLError();
	}

	void sDrawSceneMatrixFolder::init()
	{

	}

	void sDrawSceneMatrixFolder::warmExec()
	{
		exec();
	}

	void sDrawSceneMatrixFolder::exec()
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
		m_pShader->setValue("projection", projection);	// TODO: Should we send the projection/view matrices here?
		m_pShader->setValue("view", view);

		// For MotionBlur: send the previous matrix
		m_pShader->setValue("prev_projection", m_mPrevProjection);
		m_pShader->setValue("prev_view", m_mPrevView);

		// Set the other shader variable values
		m_pVars->setValues();

		// Evaluate the expression
		m_pExprPosition->Expression.value();

		// Update Matrices with instances positions, if required
		if (m_bUpdateFormulas)
			updateMatrices(false);

		// Draw instances
		int instance = 0;
		m_fCurrInsID = 0;
		m_pShader->setValue("n_total", m_fNumTotalInstances);	// Send total instances to draw to the shader

		for (const auto& model : m_pModel) {
			model->playAnimation = m_bPlayAnimation;
			if (model->playAnimation)
				model->setAnimation(m_iAnimationNumber);

			// TODO: is this required?
			model->m_matProjection = projection;
			model->m_matView = view;
			// end TODO

			for (int32_t i = 0; i < m_iNumInstancesPerModel; i++) {
				m_vCurrInsPos = glm::vec3(0);// By default our instances are in pos 0
				m_vCurrInsPosPolar = glm::vec3(0);
				m_pShader->setValue("n", m_fCurrInsID);				// Send the number of instance to the shader
				m_pShader->setValue("n_pos", m_vCurrInsPos);			// Send the instance relative position to the shader
				m_pShader->setValue("n_polar", m_vCurrInsPosPolar);	// Send the instance relative position to the shader (in polar format: x=alpha, y=beta, z=distance)

				model->m_matBaseModel = m_pInsMatrixModel[instance];

				// For MotionBlur, we send the previous model matrix
				m_pShader->setValue("prev_model", m_pmPrevModel[instance]);

				model->Draw(m_pShader, m_fAnimationTime, static_cast<uint32_t>(m_pVars->sampler2D.size()));

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

	void sDrawSceneMatrixFolder::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Folder scanned: " << m_pFolder << std::endl;
		ss << "Models found: " << m_pModel.size() << std::endl;
		ss << "Instances per model: " << m_iNumInstancesPerModel << std::endl;
		ss << "Total instances drawn: " << m_fNumTotalInstances << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawSceneMatrixFolder::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}

	void sDrawSceneMatrixFolder::updateMatrices(bool initPrevMatrix)
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

		for (int32_t i = 0; i < m_pModel.size(); i++) {
			for (int32_t j = 0; j < m_iNumInstancesPerModel; j++)
			{
				m_vCurrInsPos = glm::vec3(0); // All instances are located in pos 0 by default
				m_vCurrInsPosPolar = glm::vec3(0);
				// Evaluate the expression
				m_pExprPosition->Expression.value();

				// Copy previous model instance matrix, before model matrix changes
				m_pmPrevModel[instance] = m_pInsMatrixModel[instance];

				my_obj_model = &(m_pInsMatrixModel[instance]);
				*my_obj_model = matrixModel;
				*my_obj_model = glm::translate(*my_obj_model, m_vCurrInsPos);

				// Now render the instance using the "model_ref" as a model matrix start position
				*my_obj_model = glm::translate(*my_obj_model, m_vCurrInsTranslation);
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.x), glm::vec3(1, 0, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.y), glm::vec3(0, 1, 0));
				*my_obj_model = glm::rotate(*my_obj_model, glm::radians(m_vCurrInsRotation.z), glm::vec3(0, 0, 1));
				*my_obj_model = glm::scale(*my_obj_model, m_vCurrInsScale);

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