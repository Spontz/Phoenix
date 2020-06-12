#include "main.h"
#include "core/shadervars.h"

// ******************************************************************

typedef struct {
	int			model_ref;	// Reference model to be use to store positions
	int			model;		// Model to draw
	int			shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;
	int			playAnimation;		// Do we want to play the animation?
	int			AnimationNumber;	// Number of animation to play
	float		AnimationTime;		// Animation time (in seconds)

	float		n;			// Object to draw
	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;

	// Previous model, projection and view matrix, for being used in effects like motion blur
	std::vector<glm::mat4>	*prev_model;		// The model needs to be stored on a vector because we need to store the previous model matrix of each object
	glm::mat4				prev_projection;
	glm::mat4				prev_view;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
} drawSceneMatrix_section;

static drawSceneMatrix_section *local;

// ******************************************************************

sDrawSceneMatrix::sDrawSceneMatrix() {
	type = SectionType::DrawScene;
}

bool sDrawSceneMatrix::load() {
	if ((this->param.size() != 4) || (this->strings.size() != 8)) {
		LOG->Error("DrawSceneMatrix [%s]: 4 param and 8 strings needed", this->identifier.c_str());
		return false;
	}

	local = (drawSceneMatrix_section*)malloc(sizeof(drawSceneMatrix_section));
	this->vars = (void*)local;


	// Depth Buffer Clearing Flag
	local->enableDepthBufferClearing = (int)this->param[0];
	local->drawWireframe= (int)this->param[1];

	// Animation parameters
	local->playAnimation = (int)this->param[2];
	local->AnimationNumber = (int)this->param[3];
	
	// Load ref. model, model and shader
	local->model_ref = DEMO->modelManager.addModel(DEMO->dataFolder + this->strings[0]);
	local->model = DEMO->modelManager.addModel(DEMO->dataFolder + this->strings[1]);
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[2], DEMO->dataFolder + this->strings[3]);
	if (local->model_ref < 0 || local->model < 0 || local->shader < 0)
		return false;

	// Calculate the number of matrices that we need to store
	Model *my_model_ref;
	my_model_ref = DEMO->modelManager.model[local->model_ref];
	int num_matrices = 0;
	for (int i = 0; i < my_model_ref->meshes.size(); i++)
	{
		num_matrices += (int)my_model_ref->meshes[i].unique_vertices_pos.size();
	}
	local->prev_model = new std::vector<glm::mat4>;
	local->prev_model->resize(num_matrices);

	// Load model properties
	Model *my_model;
	my_model = DEMO->modelManager.model[local->model];
	my_model->playAnimation = local->playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(local->AnimationNumber);


	local->exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 4; i < strings.size(); i++)
		local->exprPosition->expression += this->strings[i];

	local->exprPosition->SymbolTable.add_variable("aTime", local->AnimationTime);
	local->exprPosition->SymbolTable.add_variable("n", local->n);
	local->exprPosition->SymbolTable.add_variable("tx", local->translation.x);
	local->exprPosition->SymbolTable.add_variable("ty", local->translation.y);
	local->exprPosition->SymbolTable.add_variable("tz", local->translation.z);
	local->exprPosition->SymbolTable.add_variable("rx", local->rotation.x);
	local->exprPosition->SymbolTable.add_variable("ry", local->rotation.y);
	local->exprPosition->SymbolTable.add_variable("rz", local->rotation.z);
	local->exprPosition->SymbolTable.add_variable("sx", local->scale.x);
	local->exprPosition->SymbolTable.add_variable("sy", local->scale.y);
	local->exprPosition->SymbolTable.add_variable("sz", local->scale.z);
	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	if (!local->exprPosition->compileFormula())
		return false;

	// Create Shader variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	local->vars = new ShaderVars(this, my_shader);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->vars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	local->vars->setValues();

	return true;
}

void sDrawSceneMatrix::init() {
	
}

void sDrawSceneMatrix::exec() {
	local = (drawSceneMatrix_section *)this->vars;

	Model *my_model_ref = DEMO->modelManager.model[local->model_ref];
	Model *my_model = DEMO->modelManager.model[local->model];
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	
	// Start evaluating blending
	EvalBlendingStart();

	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (local->enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);

	// Set model properties
	my_model->playAnimation = local->playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(local->AnimationNumber);

	// Load shader
	my_shader->use();

	// For ShadowMapping
	my_shader->setValue("lightSpaceMatrix", DEMO->lightManager.light[0]->spaceMatrix);
	// End ShadowMapping

	// view/projection transformations
	glm::mat4 projection = glm::perspective(
		glm::radians(DEMO->camera->Zoom),
		//GLDRV->GetCurrentViewport().GetAspectRatio(),
		GLDRV->GetFramebufferViewport().GetAspectRatio(),
		0.1f, 10000.0f
	);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shader->setValue("projection", projection);
	my_shader->setValue("view", view);
	// For MotionBlur: send the previous matrix
	my_shader->setValue("prev_projection", local->prev_projection);
	my_shader->setValue("prev_view", local->prev_view);

	// Set the other shader variable values
	local->vars->setValues();

	// Set the position of the reference model
	glm::mat4 model;
	local->n = 0;
	int object = 0;
	for (int i = 0; i < my_model_ref->meshes.size(); i++)
	{
		for (int j = 0; j < my_model_ref->meshes[i].unique_vertices_pos.size(); j++)
		{
			// Evaluate the expression
			local->exprPosition->Expression.value();
			my_shader->setValue("n", local->n); // we send also the number of object to the shader

			model = glm::mat4(1.0f);
			model = glm::translate(model, my_model_ref->meshes[i].unique_vertices_pos[j]);

			// Now render the object using the "model_ref" as a model matrix start position
			model = glm::translate(model, local->translation);
			model = glm::rotate(model, glm::radians(local->rotation.x), glm::vec3(1, 0, 0));
			model = glm::rotate(model, glm::radians(local->rotation.y), glm::vec3(0, 1, 0));
			model = glm::rotate(model, glm::radians(local->rotation.z), glm::vec3(0, 0, 1));
			model = glm::scale(model, local->scale);
			my_model->modelTransform = model;

			// For MotionBlur, we send the previous model matrix, and then store it for later use
			my_shader->setValue("prev_model", local->prev_model[0][object]);
			local->prev_model[0][object] = model;

			my_model->Draw(my_shader->ID, local->AnimationTime);

			object++; 
			local->n = (float)object;
		}
	}
	
	// For MotionBlur: store the previous matrix
	local->prev_projection = projection;
	local->prev_view = view;

	glUseProgram(0);
	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sDrawSceneMatrix::end() {
	
}

std::string sDrawSceneMatrix::debug()
{
	local = (drawSceneMatrix_section *)this->vars;

	Model *my_model_ref = DEMO->modelManager.model[local->model_ref];
	Model *my_model = DEMO->modelManager.model[local->model];

	std::string msg;
	msg = "[ drawSceneMatrix id: " + this-> identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " Matrix file: " + my_model_ref->filename + "\n";
	msg += " file: " + my_model->filename + "\n";
	msg += " objects drawn: " + std::to_string((int)(local->n)) + "\n";
	msg += " meshes in each scene: " + std::to_string(my_model->meshes.size()) + "\n";
	return msg;
}
