#include "main.h"

// ******************************************************************

typedef struct {
	int			model;
	int			shader;
	int			enableDepthBufferClearing;
	
	//tExpression evalPositioning;			// A equation containing the calculations used to position the object
	//tExpression evalSources;				// A equation containing the calculations for iterative object renderings
} objectShader_section;

static objectShader_section *local;

// ******************************************************************

sObjectShader::sObjectShader()
{
	type = SectionType::ObjectShader;
}

void sObjectShader::load() {
	string s_demo = DEMO->demoDir;
	// script validation - TODO: Put this on a common function from "section"
	if ((this->paramNum != 1) || (this->stringNum < 3)) {
		LOG->Error("ObjectShader [%s]: 1 param and 10 strings needed", this->identifier.c_str());
		return;
	}

	local = (objectShader_section*)malloc(sizeof(objectShader_section));
	this->vars = (void*)local;


	// Depth Buffer Clearing Flag
	local->enableDepthBufferClearing = (int)this->param[0];
	

	// Load model and shader
	local->model = DEMO->modelManager.addModel(s_demo + this->strings[0]);
	local->shader = DEMO->shaderManager.addShader(s_demo + this->strings[1], s_demo + this->strings[2]);

	// TODO: Load positions - expr_eval needed
}

void sObjectShader::init() {
	
}

void sObjectShader::exec() {
	local = (objectShader_section *)this->vars;

	Model *my_model = DEMO->modelManager.model[local->model];
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (local->enableDepthBufferClearing == 1)
		glClear(GL_DEPTH_BUFFER_BIT);
	my_shader->use();
	// Set the color
	my_shader->setValue("Color", glm::vec3(sin(DEMO->runTime), cos(DEMO->runTime), sin(DEMO->runTime / 2.0f)));

	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 100.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shader->setValue("projection", projection);
	my_shader->setValue("view", view);


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, -10)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
	model = glm::rotate(model, DEMO->runTime * glm::radians(-90.0f), glm::vec3(0, 1, 0));
	my_shader->setValue("model", model);

	my_model->Draw(*my_shader);

	//DEMO->text->RenderText("hola", 0, -1, 1, glm::vec3(1, 0, 0));

	//DEMO->text->RenderText("hola", sin(DEMO->runTime), cos(DEMO->runTime), sin(DEMO->runTime), glm::vec3(1, 0, 0));
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//DEMO->text->RenderText("hola New", -0.5f*cos(DEMO->runTime), -0.5f*sin(DEMO->runTime), 0.4f, glm::vec3(1, 0, 0));
	//DEMO->text->RenderText("AbraCadraBRA", -0.8f, 0, 0.4f, glm::vec3(1, 0, 0));
}

void sObjectShader::end() {
	
}
