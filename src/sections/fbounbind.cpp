#include "main.h"

typedef struct {
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
} fbounbind_section;

static fbounbind_section *local;

// ******************************************************************

sFboUnbind::sFboUnbind() {
	type = SectionType::FboUnbind;
}

bool sFboUnbind::load() {
	// script validation
	if (this->paramNum < 2) {
		LOG->Error("FboUnbind [%s]: 2 params are needed: clear the screen buffer, clear depth buffer", this->identifier.c_str());
		return false;
	}

	local = (fbounbind_section*) malloc(sizeof(fbounbind_section));
	this->vars = (void *)local;

	// load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	return true;
}

void sFboUnbind::init() {
	
}

void sFboUnbind::exec() {

	local = (fbounbind_section*)this->vars;

	// Unbind the fbo, and restore viewport
	DEMO->fboManager.unbind();

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
}

void sFboUnbind::end() {
	
}
