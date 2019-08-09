#include "main.h"
#include "core/shadervars.h"

// ******************************************************************

typedef struct {
	int			model;
	int			shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;

	float	tx,ty,tz;// Translation
	float	rx,ry,rz;// Rotation
	float	sx,sy,sz;// Scale
	
	// Previous model, projection and view matrix, for being used in effects like motion blur
	glm::mat4	prev_model;
	glm::mat4	prev_projection;
	glm::mat4	prev_view;

	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
} objectShader_section;

static objectShader_section *local;

// ******************************************************************

sObjectShader::sObjectShader() {
	type = SectionType::ObjectShader;
}

bool sObjectShader::load() {
	if ((this->param.size() != 2) || (this->strings.size() != 6)) {
		LOG->Error("ObjectShader [%s]: 2 param and 6 strings needed", this->identifier.c_str());
		return false;
	}

	local = (objectShader_section*)malloc(sizeof(objectShader_section));
	this->vars = (void*)local;


	// Depth Buffer Clearing Flag
	local->enableDepthBufferClearing = (int)this->param[0];
	local->drawWireframe= (int)this->param[1];
	

	// Load model and shader
	local->model = DEMO->modelManager.addModel(DEMO->dataFolder + this->strings[0]);
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1], DEMO->dataFolder + this->strings[2]);
	if (local->model < 0 || local->shader < 0)
		return false;

	local->exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	local->exprPosition->expression = this->strings[3] + this->strings[4] + this->strings[5]; // Concatenate the 3 positioning strings (position+rotation+scale)
	local->exprPosition->SymbolTable.add_variable("tx", local->tx);
	local->exprPosition->SymbolTable.add_variable("ty", local->ty);
	local->exprPosition->SymbolTable.add_variable("tz", local->tz);
	local->exprPosition->SymbolTable.add_variable("rx", local->rx);
	local->exprPosition->SymbolTable.add_variable("ry", local->ry);
	local->exprPosition->SymbolTable.add_variable("rz", local->rz);
	local->exprPosition->SymbolTable.add_variable("sx", local->sx);
	local->exprPosition->SymbolTable.add_variable("sy", local->sy);
	local->exprPosition->SymbolTable.add_variable("sz", local->sz);
	local->exprPosition->Expression.register_symbol_table(local->exprPosition->SymbolTable);
	local->exprPosition->compileFormula();

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
	local->vars->setValues(true);

	return true;
}

void sObjectShader::init() {
	
}

void sObjectShader::exec() {
	local = (objectShader_section *)this->vars;

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
	my_shader->use();

	// For ShadowMapping
	my_shader->setValue("lightSpaceMatrix", DEMO->lightManager.light[0]->spaceMatrix);
	
	// End ShadowMapping

	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 10000.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shader->setValue("projection", projection);
	my_shader->setValue("view", view);


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(local->tx, local->ty, local->tz));
	model = glm::rotate(model, glm::radians(local->rx), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(local->ry), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(local->rz), glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(local->sx, local->sy, local->sz));
	my_shader->setValue("model", model);

	// MotionBlur
	my_shader->setValue("prev_projection", local->prev_projection);
	my_shader->setValue("prev_view", local->prev_view);
	my_shader->setValue("prev_model", local->prev_model);

	local->prev_projection = projection;
	local->prev_view = view;
	local->prev_model = model;
	// End MotionBlur


	// Set the values
	local->vars->setValues(false);

	my_model->Draw(*my_shader);

	
	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// End evaluating blending
	EvalBlendingEnd();
}

void sObjectShader::end() {
	
}
