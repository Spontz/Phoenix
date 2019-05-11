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


void sRenderFbo::load() {
	// script validation
	if (this->paramNum < 4) {
		LOG->Error("RenderFbo [%s]: 4 params are needed: fbo to use, clear the screen buffer, clear depth buffer and fullscreen", this->identifier.c_str());
		return;
	}
	// Check for the right kind of parameters
	if ((this->param[0] < -1.0f) || (this->param[0] > (float)FBO_BUFFERS)) {
		LOG->Error("RenderFbo [%s]: Invalid texture fbo number: %i", this->identifier.c_str(), this->param[0]);
		return;
	}

	local = (renderfbo_section*) malloc(sizeof(renderfbo_section));
	this->vars = (void *)local;

	// load parameters
	local->fbo = (int)this->param[0];
	local->clearScreen = (int)this->param[1];
	local->clearDepth = (int)this->param[2];
	local->fullscreen = (int)this->param[3];

}

void sRenderFbo::init() {
	
}

void sRenderFbo::exec() {
	local = (renderfbo_section*)this->vars;

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	glDisable(GL_DEPTH_TEST);
	{
		if (this->hasBlend)
		{
			glBlendFunc(this->sfactor, this->dfactor);
			glEnable(GL_BLEND);
		}
		
		RES->Draw_Obj_QuadFBO(local->fbo);
		
		if (this->hasBlend)
			glDisable(GL_BLEND);
	}
	glEnable(GL_DEPTH_TEST);
}

void sRenderFbo::end() {
	
}
