#include "main.h"

sGLSLShaderBind::sGLSLShaderBind()
{
	type = SectionType::GLSLShaderBind;
}

Model   * mesh;
Shader  * shader;
Texture * texture;

void sGLSLShaderBind::load() {
	LOG->Info(LOG_HIGH, "  > BIND LOAD has been called!");
	string s_demo = DEMO->demoDir;
	string s_file = s_demo + "/pool/models/alliance/alliance.obj";

	mesh = new Model(s_file);

	// Create and apply basic shader
	string s_vert = s_demo + "/pool/shaders/basic/basic.vert";
	string s_frag = s_demo + "/pool/shaders/basic/basic.frag";
	shader = new Shader(s_vert, s_frag);
	shader->apply();

	shader->setUniformMatrix4fv("world", GLDRV->world_matrix);
	shader->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(GLDRV->world_matrix))));
	shader->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);

	shader->setUniform3fv("cam_pos", GLDRV->cam_position);

	// Load texture
	texture = new Texture();
	s_file = s_demo + "/pool/models/alliance/alliance.png";
	texture->load(s_file);
	texture->bind();
}

void sGLSLShaderBind::init() {
	LOG->Info(LOG_LOW, "  > BIND INIT has been called!");
	if (GLDRV->width >= 1 && GLDRV->height >= 1) {
		GLDRV->projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(GLDRV->width), float(GLDRV->height), 0.1f, 10.0f);
		if (shader != nullptr) {
			shader->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);
		}
	}
}

void sGLSLShaderBind::exec() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Draw our triangle */
	GLDRV->world_matrix = glm::rotate(glm::mat4(1.0f), DEMO->runTime * glm::radians(-90.0f), glm::vec3(0, 1, 0));

	shader->setUniformMatrix4fv("world", GLDRV->world_matrix);
	shader->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(GLDRV->world_matrix))));

	shader->apply();
	texture->bind();
	mesh->Draw();
}



void sGLSLShaderBind::end() {
	LOG->Info(LOG_HIGH, "  > BIND END has been called!");

}
