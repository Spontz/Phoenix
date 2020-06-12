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
	if (this->param.size() != 2) {
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

	// Unbind the fbo
	DEMO->fboManager.unbind(local->clearScreen, local->clearDepth);
}

void sFboUnbind::end() {
	
}

std::string sFboUnbind::debug() {
	return "[ fboUnbind id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
