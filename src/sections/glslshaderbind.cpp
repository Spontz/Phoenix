#include "main.h"

sGLSLShaderBind::sGLSLShaderBind()
{
	type = SectionType::GLSLShaderBind;
}

int model = -1;
int shader = -1;

void sGLSLShaderBind::load() {
	string s_demo = DEMO->demoDir;

	// Load model, shader and texture
	model = DEMO->modelManager.addModel(s_demo + "/pool/models/nanosuit/nanosuit.obj");
	shader = DEMO->shaderManager.addShader(s_demo + "/pool/shaders/basic/basic.vert", s_demo + "/pool/shaders/basic/basic.frag");
}

void sGLSLShaderBind::init() {
	
}

void sGLSLShaderBind::exec() {
	
	Shader *my_shad = DEMO->shaderManager.shader[shader];
	Model *my_model = DEMO->modelManager.model[model];

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	my_shad->use();
	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 100.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shad->setValue("projection", projection);
	my_shad->setValue("view", view);


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	model = glm::rotate(model, DEMO->runTime * glm::radians(-90.0f), glm::vec3(0, 1, 0));
	my_shad->setValue("model", model);

	//my_model->Draw(*my_shad);

	//DEMO->text->RenderText("hola", 0, -1, 1, glm::vec3(1, 0, 0));
	//DEMO->text->RenderText("hola", sin(DEMO->runTime), cos(DEMO->runTime), sin(DEMO->runTime), glm::vec3(1, 0, 0));
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//DEMO->text->RenderText("hola New", -0.5f*cos(DEMO->runTime), -0.5f*sin(DEMO->runTime), 0.4f, glm::vec3(1, 0, 0));
	//DEMO->text->RenderText("AbraCadraBRA", -0.8f, 0, 0.4f, glm::vec3(1, 0, 0));
}

void sGLSLShaderBind::end() {
	
}
