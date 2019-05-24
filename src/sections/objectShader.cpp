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
	
	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
} objectShader_section;

static objectShader_section *local;

// ******************************************************************

sObjectShader::sObjectShader() {
	type = SectionType::ObjectShader;
}

bool sObjectShader::load() {
	string s_demo = DEMO->demoDir;
	if ((this->paramNum != 2) || (this->stringNum < 6)) {
		LOG->Error("ObjectShader [%s]: 2 param and 6 strings needed", this->identifier.c_str());
		return false;
	}

	local = (objectShader_section*)malloc(sizeof(objectShader_section));
	this->vars = (void*)local;


	// Depth Buffer Clearing Flag
	local->enableDepthBufferClearing = (int)this->param[0];
	local->drawWireframe= (int)this->param[1];
	

	// Load model and shader
	local->model = DEMO->modelManager.addModel(s_demo + this->strings[0]);
	local->shader = DEMO->shaderManager.addShader(s_demo + this->strings[1], s_demo + this->strings[2]);
	if (local->model < 0 || local->shader < 0)
		return false;

	local->exprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	local->exprPosition->expression = std::string(this->strings[3]) + this->strings[4] + this->strings[5]; // Concatenate the 3 positioning strings (position+rotation+scale)
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

	// Read any shader variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	local->vars = new ShaderVars(this, my_shader);

	// Read the any other shader variables
	for (int i = 6; i < this->stringNum; i++) {
		local->vars->ReadString(this->strings[i].c_str());
	}
	return true;
}

void sObjectShader::init() {
	
}

void sObjectShader::exec() {
	int i;
	varFloat*		vfloat;
	varVec2*		vec2;
	varVec3*		vec3;
	varVec4*		vec4;
	varSampler2D*	sampler2D;

	local = (objectShader_section *)this->vars;

	Model *my_model = DEMO->modelManager.model[local->model];
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	
	// Evaluate the expression
	local->exprPosition->Expression.value();

	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (local->enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);
	my_shader->use();

	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 100.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shader->setValue("projection", projection);
	my_shader->setValue("view", view);


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(local->tx, local->ty, local->tz));
	model = glm::scale(model, glm::vec3(local->sx, local->sy, local->sz));
	model = glm::rotate(model, (float)local->rx, glm::vec3(1, 0, 0));
	model = glm::rotate(model, (float)local->ry, glm::vec3(0, 1, 0));
	model = glm::rotate(model, (float)local->rz, glm::vec3(0, 0, 1));
	my_shader->setValue("model", model);

	// Set any other value to shader
	for (i = 0; i < local->vars->vfloat.size(); i++) {
		vfloat = local->vars->vfloat[i];
		vfloat->eva->Expression.value();
		my_shader->setValue(vfloat->name, vfloat->value);
	}

	for (i = 0; i < local->vars->vec2.size(); i++) {
		vec2 = local->vars->vec2[i];
		vec2->eva->Expression.value();
		my_shader->setValue(vec2->name, glm::vec2(vec2->value[0], vec2->value[1]));
	}

	for (i = 0; i < local->vars->vec3.size(); i++) {
		vec3 = local->vars->vec3[i];
		vec3->eva->Expression.value();
		my_shader->setValue(vec3->name, glm::vec3(vec3->value[0], vec3->value[1], vec3->value[2]));
	}

	for (i = 0; i < local->vars->vec4.size(); i++) {
		vec4 = local->vars->vec4[i];
		vec4->eva->Expression.value();
		my_shader->setValue(vec4->name, glm::vec4(vec4->value[0], vec4->value[1], vec4->value[2], vec4->value[3]));
	}

	for (i = (int)local->vars->sampler2D.size() - 1; i >= 0; i--) {
		sampler2D = local->vars->sampler2D[i];
		my_shader->setValue(sampler2D->name, sampler2D->texGLid);
	}

	my_model->Draw(*my_shader);

	
	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void sObjectShader::end() {
	
}
