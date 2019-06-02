#include "main.h"
#include "core/shadervars.h"

typedef struct {
	int				fbo;
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	int				shader;			// Depth shader
	//ShaderVars		*vars;			// Depth shader vars

	glm::vec3		light_pos;		// Light position
	glm::vec3		light_lookAt;	// Light look at vector

	//mathDriver		*exprPosition;	// A equation containing the calculations to position the light

} fbodepthbind_section;

static fbodepthbind_section *local;

// ******************************************************************

sFboDepthBind::sFboDepthBind() {
	type = SectionType::FboDepthBind;
}

bool sFboDepthBind::load() {

	string s_demo = DEMO->demoDir;

	// script validation
	if (this->param.size() != 3) {
		LOG->Error("FboDepthBind [%s]: 3 params are needed: DEPTH fbo to use, clear the screen buffer, clear depth buffer", this->identifier.c_str());
		return false;
	}

	if (this->strings.size() != 2) {
		LOG->Error("FboDepthBind [%s]: 2 strings are needed: vertex and fragment shader files", this->identifier.c_str());
		return false;
	}

	local = (fbodepthbind_section*) malloc(sizeof(fbodepthbind_section));
	this->vars = (void *)local;

	// load parameters
	local->fbo = (int)this->param[0];
	local->clearScreen = (int)this->param[1];
	local->clearDepth = (int)this->param[2];

	if (local->fbo >= DEMO->fboManager.fbo.size()) {
		LOG->Error("FboDepthBind [%s]: The fbo number %i cannot be accessed, check graphics.spo file", this->identifier.c_str(), local->fbo);
		return false;
	}

	if (DEMO->fboManager.fbo[local->fbo]->engineFormat != "DEPTH") {
		LOG->Error("FboDepthBind [%s]: The fbo %i has no DEPTH format, format is: %s", this->identifier.c_str(), local->fbo, DEMO->fboManager.fbo[local->fbo]->engineFormat.c_str());
		return false;
	}

	// load Shader
	local->shader = DEMO->shaderManager.addShader(s_demo + this->strings[0], s_demo + this->strings[1]);
	if (local->shader == -1)
		return false;

	return true;
}

void sFboDepthBind::init() {
	
}

void sFboDepthBind::exec() {

	local = (fbodepthbind_section*)this->vars;
	if (local->fbo == -1)
		return;
	if (local->shader == -1)
		return;

	// Choose the proper program shader
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	
	// Put the camera into the light position
	//glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 70.5f;

	DEMO->light->Position.x = 10 * sin(12.0f);// this->runTime);
	DEMO->light->Position.y = 10 * cos(10.0f);// this->runTime);

	//lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);				// Switch to ortogonal view TODO: Review this numbers
	//lightView = glm::lookAt(DEMO->light->Position, DEMO->light->lookAt, glm::vec3(0.0, 1.0, 0.0));	// View from the light perspective
	//lightSpaceMatrix = lightProjection * lightView;

	DEMO->light->CalcSpaceMatrix(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
	glm::mat4 lsm = glm::mat4(DEMO->light->SpaceMatrix);
	
	// Enable the buffer in which we are going to paint
	DEMO->fboManager.bind(local->fbo);
	// Send to the shader the variables like the light position
	//my_shader->setValue("lightSpaceMatrix", lsm);

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen)	glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
}

void sFboDepthBind::end() {
	
}
