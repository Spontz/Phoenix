#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/shadervars.h"

struct sDrawSceneMatrix : public Section {
public:
	sDrawSceneMatrix();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	Model*		model_ref;	// Reference model to be use to store positions
	Model*		model;		// Model to draw
	Shader*		shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;
	int			playAnimation;		// Do we want to play the animation?
	int			AnimationNumber;	// Number of animation to play
	float		AnimationTime;		// Animation time (in seconds)

	float		m_numObjects;		// Total number of object to draw
	float		m_cObjID;			// Current object to draw
	glm::vec3	m_cObjPos;			// Current position of our object
	glm::vec3	m_cObjPosPolar;		// Current position of our object (in polar coordinates)
	glm::vec3	m_cObjTranslation;	// Current object translation
	glm::vec3	m_cObjRotation;		// Current object rotation
	glm::vec3	m_cObjScale;		// Current object scale

	glm::vec3	m_mObjTranslation;	// Matrix object translation
	glm::vec3	m_mObjRotation;		// Matrix object rotation
	glm::vec3	m_mObjScale;		// Matrix object scale

	// Previous model, projection and view matrix, for being used in effects like motion blur
	std::vector<glm::mat4>	*prev_model;		// The model needs to be stored on a vector because we need to store the previous model matrix of each object
	glm::mat4				prev_projection;
	glm::mat4				prev_view;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
};

// ******************************************************************

Section* instance_drawSceneMatrix() {
	return new sDrawSceneMatrix();
}

sDrawSceneMatrix::sDrawSceneMatrix()
	:
	model_ref(nullptr),
	model(nullptr),
	shader(nullptr),
	enableDepthBufferClearing(1),
	drawWireframe(0),
	playAnimation(0),
	AnimationNumber(0),
	AnimationTime(0),
	m_numObjects(0),
	m_cObjID(0),
	m_cObjPos({0,0,0}),
	m_cObjPosPolar({ 0,0,0 }),
	m_cObjTranslation({ 0,0,0 }),
	m_cObjRotation({ 0,0,0 }),
	m_cObjScale({ 1,1,1 }),
	m_mObjTranslation({ 0,0,0 }),
	m_mObjRotation({ 0,0,0 }),
	m_mObjScale({ 1,1,1 }),
	prev_model (nullptr),
	prev_projection(glm::mat4(1.0f)),
	prev_view(glm::mat4(1.0f)),
	exprPosition(nullptr),
	vars (nullptr)
{
	type = SectionType::DrawSceneMatrix;
}

bool sDrawSceneMatrix::load() {
	if ((param.size() != 4) || (strings.size() < 7)) {
		LOG->Error("DrawSceneMatrix [%s]: 4 param (Enable Depth buffer, enable wireframe, enable animation and animation number) and 7 strings needed", identifier.c_str());
		return false;
	}

	// Load default values
	AnimationNumber = 0;
	playAnimation = false;
	AnimationTime = 0;

	// Depth Buffer Clearing Flag
	enableDepthBufferClearing = (int)param[0];
	drawWireframe= (int)param[1];

	// Animation parameters
	playAnimation = (int)param[2];
	AnimationNumber = (int)param[3];
	
	// Load ref. model, model and shader
	model_ref = m_demo.modelManager.addModel(m_demo.dataFolder + strings[0]);
	model = m_demo.modelManager.addModel(m_demo.dataFolder + strings[1]);
	if (!model_ref || model < 0)
		return false;

	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[2]);
	if (!shader)
		return false;

	// Calculate the number of matrices that we need to store
	int num_matrices = 0;
	for (int i = 0; i < model_ref->meshes.size(); i++)
	{
		num_matrices += (int)model_ref->meshes[i].unique_vertices_pos.size();
	}
	m_numObjects = (float)num_matrices; // Number of objects to draw is the total amount of unique_vertices to draw
	prev_model = new std::vector<glm::mat4>;
	prev_model->resize(num_matrices);

	// Load model properties
	model->playAnimation = playAnimation;
	if (model->playAnimation)
		model->setAnimation(AnimationNumber);


	exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 3; i < strings.size(); i++)
		exprPosition->expression += strings[i];

	exprPosition->SymbolTable.add_variable("aTime", AnimationTime);
	exprPosition->SymbolTable.add_variable("n", m_cObjID);
	exprPosition->SymbolTable.add_variable("n_total", m_numObjects);
	exprPosition->SymbolTable.add_variable("x", m_cObjPos.x);
	exprPosition->SymbolTable.add_variable("y", m_cObjPos.y);
	exprPosition->SymbolTable.add_variable("z", m_cObjPos.z);
	exprPosition->SymbolTable.add_variable("a", m_cObjPosPolar.x);
	exprPosition->SymbolTable.add_variable("b", m_cObjPosPolar.y);
	exprPosition->SymbolTable.add_variable("r", m_cObjPosPolar.z);
	exprPosition->SymbolTable.add_variable("tx", m_cObjTranslation.x);
	exprPosition->SymbolTable.add_variable("ty", m_cObjTranslation.y);
	exprPosition->SymbolTable.add_variable("tz", m_cObjTranslation.z);
	exprPosition->SymbolTable.add_variable("rx", m_cObjRotation.x);
	exprPosition->SymbolTable.add_variable("ry", m_cObjRotation.y);
	exprPosition->SymbolTable.add_variable("rz", m_cObjRotation.z);
	exprPosition->SymbolTable.add_variable("sx", m_cObjScale.x);
	exprPosition->SymbolTable.add_variable("sy", m_cObjScale.y);
	exprPosition->SymbolTable.add_variable("sz", m_cObjScale.z);
	exprPosition->SymbolTable.add_variable("m_tx", m_mObjTranslation.x);
	exprPosition->SymbolTable.add_variable("m_ty", m_mObjTranslation.y);
	exprPosition->SymbolTable.add_variable("m_tz", m_mObjTranslation.z);
	exprPosition->SymbolTable.add_variable("m_rx", m_mObjRotation.x);
	exprPosition->SymbolTable.add_variable("m_ry", m_mObjRotation.y);
	exprPosition->SymbolTable.add_variable("m_rz", m_mObjRotation.z);
	exprPosition->SymbolTable.add_variable("m_sx", m_mObjScale.x);
	exprPosition->SymbolTable.add_variable("m_sy", m_mObjScale.y);
	exprPosition->SymbolTable.add_variable("m_sz", m_mObjScale.z);

	exprPosition->Expression.register_symbol_table(exprPosition->SymbolTable);
	if (!exprPosition->compileFormula())
		return false;

	// Create Shader variables
	shader->use();
	vars = new ShaderVars(this, shader);

	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		vars->ReadString(uniform[i].c_str());
	}

	// Set shader variables values
	vars->setValues();

	return true;
}

void sDrawSceneMatrix::init() {
	
}

void sDrawSceneMatrix::exec() {

	// Start evaluating blending
	EvalBlendingStart();

	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);

	// Set model properties
	model->playAnimation = playAnimation;
	if (model->playAnimation)
		model->setAnimation(AnimationNumber);

	// Load shader
	shader->use();

	// For ShadowMapping
	shader->setValue("lightSpaceMatrix", m_demo.lightManager.light[0]->spaceMatrix);
	// End ShadowMapping

	// view/projection transformations
	glm::mat4 projection = glm::perspective(
		glm::radians(m_demo.camera->Zoom),
		//GLDRV->GetCurrentViewport().GetAspectRatio(),
		GLDRV->GetFramebufferViewport().GetAspectRatio(),
		0.1f, 10000.0f
	);
	glm::mat4 view = m_demo.camera->GetViewMatrix();
	shader->setValue("projection", projection);
	shader->setValue("view", view);
	// For MotionBlur: send the previous matrix
	shader->setValue("prev_projection", prev_projection);
	shader->setValue("prev_view", prev_view);

	// Set the other shader variable values
	vars->setValues();

	glm::mat4 matrixModel; // Model matrix to be used on matrix object
	glm::mat4 objModel; // Model matrix to be used on each object
	m_cObjID = 0;
	int object = 0;
	
	// Evaluate the expression
	exprPosition->Expression.value();
	shader->setValue("n_total", m_numObjects);	// Send total objects to draw to the shader


	matrixModel = glm::mat4(1.0f);
	matrixModel = glm::translate(matrixModel, m_mObjTranslation);
	matrixModel = glm::rotate(matrixModel, glm::radians(m_mObjRotation.x), glm::vec3(1, 0, 0));
	matrixModel = glm::rotate(matrixModel, glm::radians(m_mObjRotation.y), glm::vec3(0, 1, 0));
	matrixModel = glm::rotate(matrixModel, glm::radians(m_mObjRotation.z), glm::vec3(0, 0, 1));
	matrixModel = glm::scale(matrixModel, m_mObjScale);

	for (int i = 0; i < model_ref->meshes.size(); i++)
	{
		for (int j = 0; j < model_ref->meshes[i].unique_vertices_pos.size(); j++)
		{
			m_cObjPos = model_ref->meshes[i].unique_vertices_pos[j];
			m_cObjPosPolar = model_ref->meshes[i].unique_vertices_polar[j];
			// Evaluate the expression
			exprPosition->Expression.value();
			shader->setValue("n", m_cObjID);				// Send the number of object to the shader
			shader->setValue("n_pos", m_cObjPos);			// Send the object relative position to the shader
			shader->setValue("n_polar", m_cObjPosPolar);	// Send the object relative position to the shader (in polar format: x=alpha, y=beta, z=distance)

			objModel = matrixModel;
			objModel = glm::translate(objModel, model_ref->meshes[i].unique_vertices_pos[j]);

			// Now render the object using the "model_ref" as a model matrix start position
			objModel = glm::translate(objModel, m_cObjTranslation);
			objModel = glm::rotate(objModel, glm::radians(m_cObjRotation.x), glm::vec3(1, 0, 0));
			objModel = glm::rotate(objModel, glm::radians(m_cObjRotation.y), glm::vec3(0, 1, 0));
			objModel = glm::rotate(objModel, glm::radians(m_cObjRotation.z), glm::vec3(0, 0, 1));
			objModel = glm::scale(objModel, m_cObjScale);
			model->modelTransform = objModel;

			// For MotionBlur, we send the previous model matrix, and then store it for later use
			shader->setValue("prev_model", prev_model[0][object]);
			prev_model[0][object] = objModel;

			model->Draw(shader->ID, AnimationTime);

			object++; 
			m_cObjID = (float)object;
		}
	}
	
	// For MotionBlur: store the previous matrix
	prev_projection = projection;
	prev_view = view;

	glUseProgram(0);
	if (drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sDrawSceneMatrix::end() {
	
}

std::string sDrawSceneMatrix::debug()
{
	std::string msg;
	msg = "[ drawSceneMatrix id: " +  identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " Matrix file: " + model_ref->filename + "\n";
	msg += " objects in matrix to be drawn: " + std::to_string((int)(m_numObjects)) + "\n";
	msg += " Model file: " + model->filename + "\n";
	msg += " meshes in each model: " + std::to_string(model->meshes.size()) + "\n";
	return msg;
}
