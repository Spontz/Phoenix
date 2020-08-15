#include "main.h"
#include "core/shadervars.h"

struct sCameraAbs : public Section {

public:
	sCameraAbs();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	demokernel& demo = demokernel::GetInstance();
	int			freeCam;
	// Spline cam variables
	glm::vec3	cam_pos;
	float	cam_yaw;
	float	cam_pitch;
	float	cam_roll;
	float	cam_zoom;

	// Final cam position variables
	glm::vec3	finalCam_pos;
	float	finalCam_yaw;
	float	finalCam_pitch;
	float	finalCam_roll;
	float	finalCam_zoom;

	mathDriver* exprCamera;	// A equation containing the calculations of the camera
};

// ******************************************************************

Section* instance_cameraAbsSection() {
	return new sCameraAbs();
}


sCameraAbs::sCameraAbs() {
	type = SectionType::CameraAbsSec;
}

bool sCameraAbs::load() {
	// script validation
	if ((spline.size() != 1) || (param.size() != 1) || (strings.size() < 2)) {
		LOG->Error("Camera [%s]: 1 spline, 1 param and 2 strings needed", this->identifier.c_str());
		return false;
	}

	// Load parameters
	freeCam = static_cast<int>(this->param[0]);


	// Load the camera splines
	for (int i=0; i < spline.size(); i++) {
		if (this->spline[i]->load() == false) {
			LOG->Error("CameraSection [%s]: Spline not loaded", identifier.c_str());
			return false;
		}
	}

	// init cam modifiers
	cam_pos = glm::vec3(0);
	cam_yaw = cam_pitch = cam_zoom = cam_roll =  0.0f;

	finalCam_pos = glm::vec3(0);
	finalCam_yaw = finalCam_pitch = finalCam_zoom = finalCam_roll = 0.0f;


	// Load the camera modifiers (based in formulas)
	exprCamera = new mathDriver(this);

	for (int i = 0; i < strings.size(); i++) {
		exprCamera->expression += this->strings[i];
	}

	exprCamera->SymbolTable.add_variable("c_posX", cam_pos.x);
	exprCamera->SymbolTable.add_variable("c_posY", cam_pos.y);
	exprCamera->SymbolTable.add_variable("c_posZ", cam_pos.z);

	exprCamera->SymbolTable.add_variable("c_yaw", cam_yaw);
	exprCamera->SymbolTable.add_variable("c_pitch", cam_pitch);
	exprCamera->SymbolTable.add_variable("c_roll", cam_roll);
	exprCamera->SymbolTable.add_variable("c_zoom", cam_zoom);

	exprCamera->SymbolTable.add_variable("PosX", finalCam_pos.x);
	exprCamera->SymbolTable.add_variable("PosY", finalCam_pos.y);
	exprCamera->SymbolTable.add_variable("PosZ", finalCam_pos.z);

	exprCamera->SymbolTable.add_variable("Yaw", finalCam_yaw);
	exprCamera->SymbolTable.add_variable("Pitch", finalCam_pitch);
	exprCamera->SymbolTable.add_variable("Roll", finalCam_roll);
	exprCamera->SymbolTable.add_variable("Zoom", finalCam_zoom);

	if (!exprCamera->compileFormula())
		return false;
	exprCamera->Expression.value();

	return true;
}

void sCameraAbs::init() {
}

void sCameraAbs::exec() {
	// If freeCam is active, we do nothing
	if (freeCam)
		return;

	if (!this->spline.empty()) {
		ChanVec	new_pos;

		// Calculate the motion step of the first spline and set it to "new_pos"
		this->spline[0]->MotionCalcStep(new_pos, this->runTime);

		cam_pos = glm::vec3(new_pos[0], new_pos[1], new_pos[2]);
		cam_yaw = new_pos[6];
		cam_pitch = new_pos[7];
		cam_roll = new_pos[8];
		cam_zoom = new_pos[9];

		// apply formula modifications
		exprCamera->Expression.value();
		
		DEMO->camera->setCamera(finalCam_pos,
			glm::vec3(new_pos[3], new_pos[4], new_pos[5]),
			finalCam_yaw, finalCam_pitch, finalCam_roll, finalCam_zoom);
	}
	
	
}

void sCameraAbs::end() {
}

std::string sCameraAbs::debug() {
	return "[ cameraAbsSection id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
