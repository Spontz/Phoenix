#include "main.h"

sGLSLShaderBind::sGLSLShaderBind()
{
	type = SectionType::GLSLShaderBind;
}

int model = -1;
int shader = -1;
int texture = -1;

void sGLSLShaderBind::load() {
	LOG->Info(LOG_HIGH, "  > BIND LOAD has been called!");
	string s_demo = DEMO->demoDir;

	// Load model, shader and texture
	model = DEMO->modelManager.addModel(s_demo + "/pool/models/nanosuit/nanosuit.obj");
	shader = DEMO->shaderManager.addShader(s_demo + "/pool/shaders/basic/basic.vert", s_demo + "/pool/shaders/basic/basic.frag");
	texture = DEMO->textureManager.addTexture(s_demo + "/pool/models/alliance/alliance.png");
}

void sGLSLShaderBind::init() {
	LOG->Info(LOG_LOW, "  > BIND INIT has been called!");
/*	if (GLDRV->width >= 1 && GLDRV->height >= 1) {
		GLDRV->projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(GLDRV->width), float(GLDRV->height), 0.1f, 10.0f);
		if (shader != -1) {
			DEMO->shaderManager.shader[shader]->setMat4("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);
		}
	}
	*/
}

void sGLSLShaderBind::exec() {
	
	Texture *my_tex = DEMO->textureManager.texture[texture];
	Shader *my_shad = DEMO->shaderManager.shader[shader];
	Model *my_model = DEMO->modelManager.model[model];

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	my_shad->use();
	// view/projection transformations
	float zoom = DEMO->camera->Zoom;
	glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)GLDRV->width / (float)GLDRV->height, 0.1f, 100.0f);
	glm::mat4 view = DEMO->camera->GetViewMatrix();
	my_shad->setMat4("projection", projection);
	my_shad->setMat4("view", view);


	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	model = glm::rotate(model, DEMO->runTime * glm::radians(-90.0f), glm::vec3(0, 1, 0));
	my_shad->setMat4("model", model);

	my_model->Draw(*my_shad);
}



void sGLSLShaderBind::end() {
	LOG->Info(LOG_HIGH, "  > BIND END has been called!");
}
