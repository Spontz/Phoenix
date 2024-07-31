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
		int32_t		m_iNumCopiesPerObject = 1;		// Number of copies to draw per each object
		float		m_fNumCopiesPerObject = 1.0f;	// Number of copies to draw per each object (in float, for formulas)
		float		m_fNumTotalObjects = 0;			// Total number of object to draw

		// Matrix object positioning
		glm::vec3	m_vMatrixObjTranslation = { 0, 0, 0 };	// Matrix object translation
		glm::vec3	m_vMatrixObjRotation = { 0, 0, 0 };		// Matrix object rotation
		glm::vec3	m_vMatrixObjScale = { 1, 1, 1 };		// Matrix object scale
		glm::mat4*	m_pMatrixObjModel = nullptr;			// Model matrix for each object

		// Current object positioning
		float		m_fCurrObjID = 0;						// Current object to draw
		glm::vec3	m_vCurrObjPos = { 0, 0, 0 };			// Current position of our object
		glm::vec3	m_vCurrObjPosPolar = { 0, 0, 0 };		// Current position of our object (in polar coordinates)
		glm::vec3	m_vCurrObjTranslation = { 0, 0, 0 };	// Current object translation
		glm::vec3	m_vCurrObjRotation = { 0, 0, 0 };		// Current object rotation
		glm::vec3	m_vCurrObjScale = { 1, 1, 1 };			// Current object scale


		// Previous model, projection and view matrix, for being used in effects like motion blur
		glm::mat4*	m_pmPrevModel = nullptr;			// The model needs to be stored on a vector because we need to store the previous model matrix of each object
		glm::mat4	m_mPrevProjection = glm::mat4(1.0f);
		glm::mat4	m_mPrevView = glm::mat4(1.0f);

		std::string						m_pFolder;				// Folder to scan
		std::vector<std::string>		m_pModelFilePaths;		// Models filePath to load
		std::vector<SP_Model>			m_pModel;				// Models to load
		SP_Shader						m_pShader;
		MathDriver*						m_pExprPosition = nullptr;	// An equation containing the calculations to position the object
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
		if (m_pMatrixObjModel)
			delete[] m_pMatrixObjModel;

		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
	}

	bool sDrawSceneMatrixFolder::load()
	{
		if ((param.size() != 7) || (strings.size() < 7)) {
			Logger::error("DrawSceneMatrixFolder [{}]: 7 param (number of copies per object, do depth buffer clearing, disbale depth test, enable wireframe, update formulas on each frame, enable animation and animation number) and 6 strings needed", identifier);
			return false;
		}

		// Copies per object
		m_iNumCopiesPerObject = static_cast<int32_t>(param[0]);
		m_fNumCopiesPerObject = param[0];
		if (m_iNumCopiesPerObject == 0) {
			Logger::error("DrawSceneMatrixFolder: Number of object copies cannot be 0");
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
				m_pModel.push_back(model);
		}

		if (m_pModel.empty()) {
			Logger::error("DrawSceneMatrixFolder: No objects loaded");
			return false;
		}

		if (m_pModel.size() != m_pModelFilePaths.size()) {
			Logger::error("DrawSceneMatrixFolder: Not all objects loaded!");
			return false;
		}

		// Load shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[2]);
		if (!m_pShader)
			return false;

		// Calculate the number of matrices that we need to store
		uint32_t num_obj_copies = m_iNumCopiesPerObject * static_cast<int32_t>(m_pModel.size());
		// Object Model Matrix
		m_pMatrixObjModel = new glm::mat4[num_obj_copies];
		// Previous Object model Matrix
		m_pmPrevModel = new glm::mat4[num_obj_copies];
		
		m_fNumTotalObjects = static_cast<float>(num_obj_copies);

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
		m_pExprPosition->SymbolTable.add_variable("n", m_fCurrObjID);
		m_pExprPosition->SymbolTable.add_variable("copies", m_fNumCopiesPerObject);
		m_pExprPosition->SymbolTable.add_variable("n_total", m_fNumTotalObjects);
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

		// Update Matrices with objects positions, if required
		if (m_bUpdateFormulas)
			updateMatrices(false);

		// Draw Objects
		int object = 0;
		m_fCurrObjID = 0;
		m_pShader->setValue("n_total", m_fNumTotalObjects);	// Send total objects to draw to the shader

		for (const auto& model : m_pModel) {
			model->playAnimation = m_bPlayAnimation;
			if (model->playAnimation)
				model->setAnimation(m_iAnimationNumber);

			// TODO: is this required?
			model->m_matProjection = projection;
			model->m_matView = view;
			// end TODO

			for (int32_t i = 0; i < m_iNumCopiesPerObject; i++) {
				m_vCurrObjPos = glm::vec3(0);// By default are objects are in pos 0
				m_vCurrObjPosPolar = glm::vec3(0);
				m_pShader->setValue("n", m_fCurrObjID);				// Send the number of object to the shader
				m_pShader->setValue("n_pos", m_vCurrObjPos);			// Send the object relative position to the shader
				m_pShader->setValue("n_polar", m_vCurrObjPosPolar);	// Send the object relative position to the shader (in polar format: x=alpha, y=beta, z=distance)

				model->m_matBaseModel = m_pMatrixObjModel[object];

				// For MotionBlur, we send the previous model matrix
				m_pShader->setValue("prev_model", m_pmPrevModel[object]);

				model->Draw(m_pShader->getId(), m_fAnimationTime, static_cast<uint32_t>(m_pVars->sampler2D.size()));

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

	void sDrawSceneMatrixFolder::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Shader: " << m_pShader->getURI() << std::endl;
		ss << "Folder scanned: " << m_pFolder << std::endl;
		ss << "Objects found: " << m_pModel.size() << std::endl;
		ss << "Copies per object: " << m_iNumCopiesPerObject << std::endl;
		ss << "Total objects drawn: " << m_fNumTotalObjects << std::endl;
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

		for (int32_t i = 0; i < m_pModel.size(); i++) {
			for (int32_t j = 0; j < m_iNumCopiesPerObject; j++)
			{
				m_vCurrObjPos = glm::vec3(0); // All objects are located in pos 0 by default
				m_vCurrObjPosPolar = glm::vec3(0);
				// Evaluate the expression
				m_pExprPosition->Expression.value();

				// Copy previous model object matrix, before model matrix changes
				m_pmPrevModel[object] = m_pMatrixObjModel[object];

				my_obj_model = &(m_pMatrixObjModel[object]);
				*my_obj_model = matrixModel;
				*my_obj_model = glm::translate(*my_obj_model, m_vCurrObjPos);

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