#include "main.h"
#include "core/shadervars.h"

// ******************************************************************

typedef struct {
	int			model;
	int			shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;
	float		CameraNumber;		// Number of the camera to use (-1 = means to not use camera)
	bool		playAnimation;		// Do we want to play the animation?
	int			AnimationNumber;	// Number of animation to play
	float		AnimationTime;		// Animation time (in seconds)

	glm::vec3	translation;
	glm::vec3	rotation;
	glm::vec3	scale;
	
	// Previous model, projection and view matrix, for being used in effects like motion blur
	glm::mat4	prev_model;
	glm::mat4	prev_projection;
	glm::mat4	prev_view;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
} drawScene_section;

static drawScene_section *local;

// ******************************************************************

sDrawScene::sDrawScene() {
	type = SectionType::DrawScene;
}

bool sDrawScene::load() {
	if ((this->param.size() != 4) || (this->strings.size() < 6)) {
		LOG->Error("DrawScene [%s]: 4 param (Enable Depth buffer, enable wireframe, enable animation) and 6 strings needed", this->identifier.c_str());
		return false;
	}

	local = (drawScene_section*)malloc(sizeof(drawScene_section));
	this->vars = (void*)local;


	// Depth Buffer Clearing Flag
	local->enableDepthBufferClearing = (int)this->param[0];
	local->drawWireframe= (int)this->param[1];

	// Animation parameters
	local->playAnimation = (int)this->param[2];
	local->AnimationNumber = (int)this->param[3];
	
	// Load model and shader
	local->model = DEMO->modelManager.addModel(DEMO->dataFolder + this->strings[0]);
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1]);
	if (local->model < 0 || local->shader < 0)
		return false;

	// Load model properties
	Model *my_model;
	my_model = DEMO->modelManager.model[local->model];
	my_model->playAnimation = local->playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(local->AnimationNumber);

	local->exprPosition = new mathDriver(this);
	// Load all the other strings
	for (int i = 2; i < strings.size(); i++)
		local->exprPosition->expression += this->strings[i];

	local->exprPosition->SymbolTable.add_variable("CameraNumber", local->CameraNumber);
	local->exprPosition->SymbolTable.add_variable("aTime", local->AnimationTime);
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
	local->exprPosition->Expression.value();
	// Set the camera number
	if (local->CameraNumber < 0)
		my_model->useCamera = false;
	else
		my_model->setCamera((unsigned int)local->CameraNumber);


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

void sDrawScene::init() {
	
}

void sDrawScene::exec() {
	local = (drawScene_section *)this->vars;

	Model *my_model = DEMO->modelManager.model[local->model];
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	
	// Start evaluating blending
	EvalBlendingStart();

	// Evaluate the expression
	local->exprPosition->Expression.value();

	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (local->enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);

	// Set model properties
	my_model->playAnimation = local->playAnimation;
	if (my_model->playAnimation)
		my_model->setAnimation(local->AnimationNumber);
	if (local->CameraNumber < 0)
		my_model->useCamera = false;
	else
		my_model->setCamera((unsigned int)local->CameraNumber);

	// Load shader
	my_shader->use();

	// For ShadowMapping
	my_shader->setValue("lightSpaceMatrix", DEMO->lightManager.light[0]->spaceMatrix);
	// End ShadowMapping

	// view/projection transformations
	glm::mat4 projection = glm::perspective(
		glm::radians(DEMO->camera->Zoom),
		GLDRV->GetFramebufferViewport().GetAspectRatio(),
		0.1f, 10000.0f
	);

	my_shader->setValue("projection", projection);

	glm::mat4 view = DEMO->camera->GetViewMatrix();
	//if (local->CameraNumber < 0)
		my_shader->setValue("view", view);

	


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, local->translation);
	model = glm::rotate(model, glm::radians(local->rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(local->rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(local->rotation.z), glm::vec3(0, 0, 1));
	model = glm::scale(model, local->scale);
	my_model->modelTransform = model;

	// For MotionBlur
	my_shader->setValue("prev_projection", local->prev_projection);
	//if (local->CameraNumber < 0)
		my_shader->setValue("prev_view", local->prev_view);
	my_shader->setValue("prev_model", local->prev_model);

	local->prev_projection = projection;
	//if (local->CameraNumber < 0)
		local->prev_view = view;
	local->prev_model = model;
	// End MotionBlur

	// Set the other shader variable values
	local->vars->setValues();

	my_model->Draw(my_shader->ID, local->AnimationTime);

	glUseProgram(0);
	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sDrawScene::end() {
	
}

std::string sDrawScene::debug()
{
	local = (drawScene_section *)this->vars;

	Model *my_model = DEMO->modelManager.model[local->model];

	std::string msg;
	msg = "[ drawScene id: " + this-> identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " file: " + my_model->filename + "\n";
	msg += " meshes: " + std::to_string(my_model->meshes.size()) + "\n";
/*	for (int i=0; i<my_model->meshes.size(); i++) {
		msg += "  mesh: " + std::to_string(i) + "\n";
		msg += "    Num vertices: " + std::to_string(my_model->meshes[i].vertices.size()) + "\n";
		msg += "    Num textures: " + std::to_string(my_model->meshes[i].material.textures.size()) + "\n";
		msg += "    Color Diffuse [" +	std::to_string(my_model->meshes[i].material.colDiffuse.r) + " / " +
										std::to_string(my_model->meshes[i].material.colDiffuse.g) + " / " +
										std::to_string(my_model->meshes[i].material.colDiffuse.b) + " ] " + "\n";
		msg += "    Color Ambient [" + std::to_string(my_model->meshes[i].material.colAmbient.r) + " / " +
										std::to_string(my_model->meshes[i].material.colAmbient.g) + " / " +
										std::to_string(my_model->meshes[i].material.colAmbient.b) + " ] " + "\n";
		msg += "    Color Specular [" + std::to_string(my_model->meshes[i].material.colSpecular.r) + " / " +
										std::to_string(my_model->meshes[i].material.colSpecular.g) + " / " +
										std::to_string(my_model->meshes[i].material.colSpecular.b) + " ] " + "\n";
	}
*/	
	return msg;
}
