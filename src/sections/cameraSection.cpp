#include "main.h"
#include "core/shadervars.h"

typedef struct {
	int			freeCam;
	glm::vec3	cam_pos;
	float		cam_yaw;
	float		cam_pitch;
	float		cam_roll;
	float		cam_zoom;

	mathDriver* exprCamera;	// A equation containing the calculations of the camera
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
	for (int i=0; i < this->spline.size(); i++) {
		if (this->spline[i]->load() == false) {
			LOG->Error("CameraSection [%s]: Spline not loaded", this->identifier.c_str());
			return false;
		}
	}

	// init cam modifiers
	local->cam_pos = glm::vec3(0);
	local->cam_yaw = local->cam_pitch = local->cam_zoom = local->cam_roll = 0.0f;

	// Load the camera modifiers (based in formulas)
	local->exprCamera = new mathDriver(this);
	local->exprCamera->expression += "var zzz:=0;"; // Hack: We set up a basic string, just in case no string is given

	for (int i = 0; i < this->strings.size(); i++) {
		local->exprCamera->expression += this->strings[i];
	}

	local->exprCamera->SymbolTable.add_variable("PosX", local->cam_pos.x);
	local->exprCamera->SymbolTable.add_variable("PosY", local->cam_pos.y);
	local->exprCamera->SymbolTable.add_variable("PosZ", local->cam_pos.z);

	local->exprCamera->SymbolTable.add_variable("Yaw", local->cam_yaw);
	local->exprCamera->SymbolTable.add_variable("Pitch", local->cam_pitch);
	local->exprCamera->SymbolTable.add_variable("Roll", local->cam_roll);
	local->exprCamera->SymbolTable.add_variable("Zoom", local->cam_zoom);


	if (!local->exprCamera->compileFormula())
		return false;
	local->exprCamera->Expression.value();


	return true;
}

void sCamera::init() {
}

void sCamera::exec() {
	local = (camera_section *)this->vars;

	// If freeCam is active, we do nothing
	if (local->freeCam)
		return;

	if (!this->spline.empty()) {
		ChanVec	new_pos;

		// Calculate the motion step of the first spline and set it to "new_pos"
		this->spline[0]->MotionCalcStep(new_pos, this->runTime);

		// apply formula modifications
		local->exprCamera->Expression.value();

		DEMO->camera->setCamera(glm::vec3(new_pos[0], new_pos[1], new_pos[2]) + local->cam_pos,
			glm::vec3(new_pos[3], new_pos[4], new_pos[5]),
			new_pos[6] + local->cam_yaw, new_pos[7] + local->cam_pitch, new_pos[8] + local->cam_roll, new_pos[9] + local->cam_zoom);
	}
	
	
}

void sCamera::end() {
}

std::string sCamera::debug() {
	return "[ cameraSection id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
