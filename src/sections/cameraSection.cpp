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
	if ((this->spline.size() != 1) || (this->param.size() != 1)) {
		LOG->Error("Camera [%s]: 1 spline and 1 param needed", this->identifier.c_str());
		return false;
	}

	local = (camera_section*)malloc(sizeof(camera_section));
	this->vars = (void *)local;

	// Load parameters
	local->freeCam = static_cast<int>(this->param[0]);


	// Load the camera splines
	for (int i = 0; i < this->spline.size(); i++) {
		this->spline[i]->load();
	}
	
	return true;
}

void sCamera::init() {
}

void sCamera::exec() {
	local = (camera_section *)this->vars;

	// TODO: We could load several splines and use one or another spline, depending on one variable,
	// so, we could be able to change the camera depending on a variable

	// If freeCam is active, we do nothing
	if (local->freeCam)
		return;

	if (!this->spline.empty()) {
		ChanVec	new_pos;

		// Calculate the motion step of the first spline and set it to "new_pos"
		this->spline[0]->MotionCalcStep(new_pos, this->runTime);
		
		DEMO->camera->setCamera(glm::vec3(new_pos[0], new_pos[1], new_pos[2]),
			glm::vec3(new_pos[3], new_pos[4], new_pos[5]),
			new_pos[6], new_pos[7], new_pos[8]);
	}
	
	
}

void sCamera::end() {
}
