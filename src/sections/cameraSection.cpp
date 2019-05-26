#include "main.h"

typedef struct {
	int		freeCam;
} camera_section;

static camera_section *local;

sCamera::sCamera() {
	type = SectionType::CameraSec;
}

bool sCamera::load() {
	// script validation
	if ((this->splineNum != 1) || (this->paramNum != 1)) {
		LOG->Error("Camera [%s]: 1 spline and 1 param needed", this->identifier.c_str());
		return false;
	}

	local = (camera_section*)malloc(sizeof(camera_section));
	this->vars = (void *)local;

	// Load the camera splines
	this->loadSplines();
	
	return true;
}

void sCamera::init() {
}

void sCamera::exec() {
	local = (camera_section *)this->vars;
	ChanVec	new_pos;
	this->spline[0]->MotionCalcStep(new_pos, this->runTime);
	
	DEMO->camera->setCamera(glm::vec3(new_pos[0], new_pos[1], new_pos[2]),
							glm::vec3(new_pos[3], new_pos[4], new_pos[5]),
							new_pos[6], new_pos[7], new_pos[8]);

	
}

void sCamera::end() {
}
