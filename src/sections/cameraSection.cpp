#include "main.h"
#include "core/shadervars.h"

struct sCamera : public Section {
public:
	sCamera();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	int			freeCam;
	glm::vec3	cam_pos;
	float		cam_yaw;
	float		cam_pitch;
	float		cam_roll;
	float		cam_zoom;

	mathDriver* exprCamera;	// A equation containing the calculations of the camera
} camera_section;

// ******************************************************************

Section* instance_camera() {
	return new sCamera();
}
sCamera::sCamera() {
	type = SectionType::CameraSec;
}

bool sCamera::load() {
	// script validation
	if ((spline.size() != 1) || (param.size() != 1)) {
		LOG->Error("Camera [%s]: 1 spline and 1 param needed", identifier.c_str());
		return false;
	}

	// Load parameters
	freeCam = static_cast<int>(param[0]);


	// Load the camera splines
	for (int i=0; i < spline.size(); i++) {
		if (spline[i]->load() == false) {
			LOG->Error("CameraSection [%s]: Spline not loaded", identifier.c_str());
			return false;
		}
	}

	// init cam modifiers
	cam_pos = glm::vec3(0);
	cam_yaw = cam_pitch = cam_zoom = cam_roll = 0.0f;

	// Load the camera modifiers (based in formulas)
	exprCamera = new mathDriver(this);
	exprCamera->expression += "var zzz:=0;"; // Hack: We set up a basic string, just in case no string is given

	for (int i = 0; i < strings.size(); i++) {
		exprCamera->expression += strings[i];
	}

	exprCamera->SymbolTable.add_variable("PosX", cam_pos.x);
	exprCamera->SymbolTable.add_variable("PosY", cam_pos.y);
	exprCamera->SymbolTable.add_variable("PosZ", cam_pos.z);

	exprCamera->SymbolTable.add_variable("Yaw", cam_yaw);
	exprCamera->SymbolTable.add_variable("Pitch", cam_pitch);
	exprCamera->SymbolTable.add_variable("Roll", cam_roll);
	exprCamera->SymbolTable.add_variable("Zoom", cam_zoom);


	if (!exprCamera->compileFormula())
		return false;
	exprCamera->Expression.value();


	return true;
}

void sCamera::init() {
}

void sCamera::exec() {
	// If freeCam is active, we do nothing
	if (freeCam)
		return;

	if (!spline.empty()) {
		ChanVec	new_pos;

		// Calculate the motion step of the first spline and set it to "new_pos"
		spline[0]->MotionCalcStep(new_pos, runTime);

		// apply formula modifications
		exprCamera->Expression.value();

		DEMO->camera->setCamera(glm::vec3(new_pos[0], new_pos[1], new_pos[2]) + cam_pos,
			glm::vec3(new_pos[3], new_pos[4], new_pos[5]),
			new_pos[6] + cam_yaw, new_pos[7] + cam_pitch, new_pos[8] + cam_roll, new_pos[9] + cam_zoom);
	}
	
	
}

void sCamera::end() {
}

std::string sCamera::debug() {
	return "[ cameraSection id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
}
