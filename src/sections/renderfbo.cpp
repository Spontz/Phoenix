#include "main.h"

typedef struct {
	int		fbo;
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
	char	fullscreen;		// fullscreen restore
} renderfbo_section;

static renderfbo_section *local;

// ******************************************************************

sRenderFbo::sRenderFbo() {
	type = SectionType::RenderFbo;
}


bool sRenderFbo::load() {
	// script validation
	if (this->param.size() != 4) {
		LOG->Error("RenderFbo [%s]: 4 params are needed: fbo to use, clear the screen buffer, clear depth buffer and fullscreen", this->identifier.c_str());
		return false;
	}
	// Check for the right kind of parameters
	if ((this->param[0] < -1.0f) || (this->param[0] > (float)FBO_BUFFERS)) {
		LOG->Error("RenderFbo [%s]: Invalid texture fbo number: %i", this->identifier.c_str(), this->param[0]);
		return false;
	}

	local = (renderfbo_section*) malloc(sizeof(renderfbo_section));
	this->vars = (void *)local;

	// load parameters
	local->fbo = (int)this->param[0];
	local->clearScreen = (int)this->param[1];
	local->clearDepth = (int)this->param[2];
	local->fullscreen = (int)this->param[3];

	return true;
}

void sRenderFbo::init() {
	
}

void sRenderFbo::exec() {
	local = (renderfbo_section*)this->vars;

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	EvalBlendingStart();

	glDisable(GL_DEPTH_TEST);
	{
		//RES->Draw_QuadFBOFS(local->fbo);
		RES->Draw_QuadFBOFS_noShader(local->fbo);
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sRenderFbo::end() {
	
}
