#include "main.h"

typedef struct {
	int		fbo;
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
} fbobind_section;

static fbobind_section *local;

// ******************************************************************

sFboBind::sFboBind() {
	type = SectionType::FboBind;
}

bool sFboBind::load() {
	// script validation
	if (this->paramNum < 3) {
		LOG->Error("FboBind [%s]: 3 params are needed: fbo to use, clear the screen buffer, clear depth buffer", this->identifier.c_str());
		return false;
	}

	local = (fbobind_section*) malloc(sizeof(fbobind_section));
	this->vars = (void *)local;

	// load parameters
	local->fbo = (int)this->param[0];
	local->clearScreen = (int)this->param[1];
	local->clearDepth = (int)this->param[2];
	return true;
}

void sFboBind::init() {
	
}

void sFboBind::exec() {

	local = (fbobind_section*)this->vars;
	if (local->fbo == -1)
		return;
	// Enable the buffer in which we are going to paint
	DEMO->fboManager.bind(local->fbo);

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen)	glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
}

void sFboBind::end() {
	
}
