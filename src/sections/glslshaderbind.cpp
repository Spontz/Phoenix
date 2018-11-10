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

	//mesh = new Model(s_file);
	// Load model
	model = DEMO->modelManager.addModel(s_demo + "/pool/models/alliance/alliance.obj");
		
	//shader = new Shader(s_vert, s_frag);
	shader = DEMO->shaderManager.addShader(s_demo + "/pool/shaders/basic/basic.vert", s_demo + "/pool/shaders/basic/basic.frag");
	DEMO->shaderManager.shader[shader]->apply();

	DEMO->shaderManager.shader[shader]->setUniformMatrix4fv("world", GLDRV->world_matrix);
	DEMO->shaderManager.shader[shader]->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(GLDRV->world_matrix))));
	DEMO->shaderManager.shader[shader]->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);

	DEMO->shaderManager.shader[shader]->setUniform3fv("cam_pos", GLDRV->cam_position);

	// Load texture
	texture = DEMO->textureManager.addTexture(s_demo + "/pool/models/alliance/alliance.png");
}

void sGLSLShaderBind::init() {
	LOG->Info(LOG_LOW, "  > BIND INIT has been called!");
	if (GLDRV->width >= 1 && GLDRV->height >= 1) {
		GLDRV->projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(GLDRV->width), float(GLDRV->height), 0.1f, 10.0f);
		if (shader != -1) {
			DEMO->shaderManager.shader[shader]->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);
		}
	}
}

void sGLSLShaderBind::exec() {
	
	// Rotate our "world"
	GLDRV->world_matrix = glm::rotate(glm::mat4(1.0f), DEMO->runTime * glm::radians(-90.0f), glm::vec3(0, 1, 0));

	DEMO->shaderManager.shader[shader]->setUniformMatrix4fv("world", GLDRV->world_matrix);
	DEMO->shaderManager.shader[shader]->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(GLDRV->world_matrix))));

	DEMO->shaderManager.shader[shader]->apply();
	DEMO->textureManager.texture[texture]->bind();
	DEMO->modelManager.model[model]->Draw();
}



void sGLSLShaderBind::end() {
	LOG->Info(LOG_HIGH, "  > BIND END has been called!");
}
