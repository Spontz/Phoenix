#include "main.h"

struct sDrawFbo : public Section {
public:
	sDrawFbo();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	int		fbo;
	int		fboAttachment;
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
};

// ******************************************************************

Section* instance_drawFbo() {
	return new sDrawFbo();
}

sDrawFbo::sDrawFbo() {
	type = SectionType::DrawFbo;
}


bool sDrawFbo::load() {
	// script validation
	if (param.size() != 4) {
		LOG->Error("DrawFbo [%s]: 4 params are needed: fbo to use and attachment, clear the screen buffer, clear depth buffer", identifier.c_str());
		return false;
	}

	// load parameters
	fbo = (int)param[0];
	fboAttachment = (int)param[1];
	clearScreen = (int)param[2];
	clearDepth = (int)param[3];

	// Check for the right parameter values
	if ((fbo < 0) || (fbo > (float)FBO_BUFFERS)) {
		LOG->Error("DrawFbo [%s]: Invalid texture fbo number: %i", identifier.c_str(), fbo);
		return false;
	}

	if ((fboAttachment < 0) || (fboAttachment > (float)GLDRV_MAX_COLOR_ATTACHMENTS)) {
		LOG->Error("DrawFbo [%s]: Invalid texture fbo attachment: %i", identifier.c_str(), fboAttachment);
		return false;
	}

	return true;
}

void sDrawFbo::init() {
	
}

void sDrawFbo::exec() {
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	EvalBlendingStart();

	glDisable(GL_DEPTH_TEST);
	{
		RES->Draw_QuadFBOFS(fbo, fboAttachment);
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawFbo::end() {
	
}

std::string sDrawFbo::debug() {
	std::string msg;
	msg = "[ drawFbo id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " fbo: " + std::to_string(fbo) + "\n";
	return msg;
}
