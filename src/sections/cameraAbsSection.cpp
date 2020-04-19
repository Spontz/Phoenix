#include "main.h"
#include "core/shadervars.h"

typedef struct {
	int			freeCam;

	// Spline cam variables
	glm::vec3	cam_pos;
	float	cam_yaw;
	float	cam_pitch;
	float	cam_zoom;

	// Final cam position variables
	glm::vec3	finalCam_pos;
	float	finalCam_yaw;
	float	finalCam_pitch;
	float	finalCam_zoom;


	mathDriver* exprCamera;	// A equation containing the calculations of the camera
} cameraAbs_section;

static cameraAbs_section *local;

sCameraAbs::sCameraAbs() {
	type = SectionType::CameraAbsSec;
}

bool sCameraAbs::load() {
	// script validation
	if ((this->spline.size() != 1) || (this->param.size() != 1) || (this->strings.size() < 2)) {
		LOG->Error("Camera [%s]: 1 spline, 1 param and 2 strings needed", this->identifier.c_str());
		return false;
	}

	local = (cameraAbs_section*)malloc(sizeof(cameraAbs_section));
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
	local->cam_yaw = local->cam_pitch = local->cam_zoom = 0.0f;

	local->finalCam_pos = glm::vec3(0);
	local->finalCam_yaw = local->finalCam_pitch = local->finalCam_zoom = 0.0f;


	// Load the camera modifiers (based in formulas)
	local->exprCamera = new mathDriver(this);

	for (int i = 0; i < this->strings.size(); i++) {
		local->exprCamera->expression += this->strings[i];
	}

	local->exprCamera->SymbolTable.add_variable("c_posX", local->cam_pos.x);
	local->exprCamera->SymbolTable.add_variable("c_posY", local->cam_pos.y);
	local->exprCamera->SymbolTable.add_variable("c_posZ", local->cam_pos.z);

	local->exprCamera->SymbolTable.add_variable("c_yaw", local->cam_yaw);
	local->exprCamera->SymbolTable.add_variable("c_pitch", local->cam_pitch);
	local->exprCamera->SymbolTable.add_variable("c_zoom", local->cam_zoom);

	local->exprCamera->SymbolTable.add_variable("PosX", local->finalCam_pos.x);
	local->exprCamera->SymbolTable.add_variable("PosY", local->finalCam_pos.y);
	local->exprCamera->SymbolTable.add_variable("PosZ", local->finalCam_pos.z);

	local->exprCamera->SymbolTable.add_variable("Yaw", local->finalCam_yaw);
	local->exprCamera->SymbolTable.add_variable("Pitch", local->finalCam_pitch);
	local->exprCamera->SymbolTable.add_variable("Zoom", local->finalCam_zoom);

	if (!local->exprCamera->compileFormula())
		return false;
	local->exprCamera->Expression.value();


	return true;
}

void sCameraAbs::init() {
}

void sCameraAbs::exec() {
	local = (cameraAbs_section *)this->vars;

	// If freeCam is active, we do nothing
	if (local->freeCam)
		return;

	if (!this->spline.empty()) {
		ChanVec	new_pos;

		// Calculate the motion step of the first spline and set it to "new_pos"
		this->spline[0]->MotionCalcStep(new_pos, this->runTime);

		local->cam_pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
		local->cam_yaw = new_pos[6];
		local->cam_pitch = new_pos[7];
		local->cam_zoom = new_pos[8];

		// apply formula modifications
		local->exprCamera->Expression.value();
		
		DEMO->camera->setCamera(local->finalCam_pos,
			glm::vec3(new_pos[3], new_pos[4], new_pos[5]),
			local->finalCam_yaw, local->finalCam_pitch, local->finalCam_zoom);
	}
	
	
}

void sCameraAbs::end() {
}

string sCameraAbs::debug() {
	return "[ cameraAbsSection id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
