#include "main.h"

typedef struct {
	int		fbo;
	int		fboAttachment;
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
} drawFbo_section;

static drawFbo_section *local;

// ******************************************************************

sDrawFbo::sDrawFbo() {
	type = SectionType::DrawFbo;
}


bool sDrawFbo::load() {
	// script validation
	if (this->param.size() != 4) {
		LOG->Error("DrawFbo [%s]: 4 params are needed: fbo to use and attachment, clear the screen buffer, clear depth buffer", this->identifier.c_str());
		return false;
	}
	

	local = (drawFbo_section*) malloc(sizeof(drawFbo_section));
	this->vars = (void *)local;

	// load parameters
	local->fbo = (int)this->param[0];
	local->fboAttachment = (int)this->param[1];
	local->clearScreen = (int)this->param[2];
	local->clearDepth = (int)this->param[3];

	// Check for the right parameter values
	if ((local->fbo < 0) || (local->fbo > (float)FBO_BUFFERS)) {
		LOG->Error("DrawFbo [%s]: Invalid texture fbo number: %i", this->identifier.c_str(), local->fbo);
		return false;
	}

	if ((local->fboAttachment < 0) || (local->fboAttachment > (float)GLDRV_MAX_COLOR_ATTACHMENTS)) {
		LOG->Error("DrawFbo [%s]: Invalid texture fbo attachment: %i", this->identifier.c_str(), local->fboAttachment);
		return false;
	}

	return true;
}

void sDrawFbo::init() {
	
}

void sDrawFbo::exec() {
	local = (drawFbo_section*)this->vars;

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	EvalBlendingStart();

	glDisable(GL_DEPTH_TEST);
	{
		RES->Draw_QuadFBOFS(local->fbo, local->fboAttachment);
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawFbo::end() {
	
}
