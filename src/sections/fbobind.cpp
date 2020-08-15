#include "main.h"

struct sFboBind : public Section{
public:
	sFboBind();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	int		fbo;
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
};

// ******************************************************************

Section* instance_fboBind() {
	return new sFboBind();
}

sFboBind::sFboBind() {
	type = SectionType::FboBind;
}

bool sFboBind::load() {
	// script validation
	if (param.size() != 3) {
		LOG->Error("FboBind [%s]: 3 params are needed: fbo to use, clear the screen buffer, clear depth buffer", identifier.c_str());
		return false;
	}

	// load parameters
	fbo = (int)param[0];
	clearScreen = (int)param[1];
	clearDepth = (int)param[2];

	if (fbo >= m_demo.fboManager.fbo.size()) {
		LOG->Error("FboBind [%s]: The fbo number %i cannot be accessed, check graphics.spo file", identifier.c_str(), fbo);
		return false;
	}

	return true;
}

void sFboBind::init() {
	
}

void sFboBind::exec() {

	if (fbo == -1)
		return;
	// Enable the buffer in which we are going to paint
	m_demo.fboManager.bind(fbo, clearScreen, clearDepth);
}

void sFboBind::end() {
	
}

std::string sFboBind::debug() {
	std::string msg;
	msg = "[ fboBind id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " fbo: " + std::to_string(fbo) + "\n";
	return msg;
}
