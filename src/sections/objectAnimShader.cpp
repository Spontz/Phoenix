#include "main.h"
#include "core/shadervars.h"
#include <glm/gtc/type_ptr.hpp>

// ******************************************************************

typedef struct {
	int			model;
	int			shader;
	int			enableDepthBufferClearing;
	int			drawWireframe;
	SkinnedMesh *my_SKmesh; // TODO: Replace this shit for a model, and use modelmanager

	float	tx,ty,tz;// Translation
	float	rx,ry,rz;// Rotation
	float	sx,sy,sz;// Scale
	
	mathDriver	*exprPosition;	// A equation containing the calculations to position the object
	ShaderVars	*vars;			// For storing any other shader variables
} objectAnimShader_section;

static objectAnimShader_section *local;

// ******************************************************************

sObjectAnimShader::sObjectAnimShader() {
	type = SectionType::ObjectAnimShader;
}

bool sObjectAnimShader::load() {
	string s_demo = DEMO->demoDir;
	if ((this->paramNum != 2) || (this->stringNum < 6)) {
		LOG->Error("ObjectShader [%s]: 2 param and 6 strings needed", this->identifier.c_str());
		return false;
	}

	local = (objectAnimShader_section*)malloc(sizeof(objectAnimShader_section));
	this->vars = (void*)local;


	// Depth Buffer Clearing Flag
	local->enableDepthBufferClearing = (int)this->param[0];
	local->drawWireframe= (int)this->param[1];
	

	// Load model and shader
	local->my_SKmesh = new SkinnedMesh();
	local->my_SKmesh->loadMesh(s_demo + this->strings[0]);
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

void sObjectAnimShader::init() {
	
}

void sObjectAnimShader::exec() {
	int i;
	varFloat*		vfloat;
	varVec2*		vec2;
	varVec3*		vec3;
	varVec4*		vec4;
	varSampler2D*	sampler2D;

	local = (objectAnimShader_section *)this->vars;

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
		// TODO: Canviar això del Active per:
		// DEMO->textureManager.active(i); --> Sha de crear el mètode a nivell de manager, perque ara està a nivell de textura i no te sentit
		glActiveTexture(GL_TEXTURE0 + sampler2D->texUnitID);
		my_shader->setValue(sampler2D->name, sampler2D->texUnitID); // El set s'ha de fer sol al principi! no cal fer-lo a cada frame!!
		glBindTexture(GL_TEXTURE_2D, sampler2D->texGLid);
	}
	// Guarrada para pasar una textura
	glUniform1i(glGetUniformLocation(my_shader->ID, "texture_diffuse1"), 0); // Pick some random texture
	DEMO->textureManager.texture[8]->active();
	DEMO->textureManager.texture[8]->bind();
	//////// End guarrada

	// Send Bones info to shader
	//vector<glm::mat4> Transforms;
	//my_SKmesh->boneTransform(0, Transforms);
	//glUniformMatrix4fv(glGetUniformLocation(my_shader->ID, "gBones"), (GLsizei)Transforms.size(), GL_FALSE, glm::value_ptr(Transforms[0]));
	local->my_SKmesh->setBoneTransformations(my_shader->ID, this->runTime);
	local->my_SKmesh->render(my_shader->ID);
	//std::vector<glm::mat4> transforms = my_model->boneTransform("./data/pool/models/anim/anim.md5anim", 0);// this->runTime);
	//glUniformMatrix4fv(glGetUniformLocation(my_shader->ID, "gBones"), (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
	
	if (local->drawWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// End evaluating blending
	EvalBlendingEnd();
}

void sObjectAnimShader::end() {
	
}
