#include "main.h"

struct sFboUnbind : public Section {
public:
	sFboUnbind();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	char	clearScreen;	// Clear Screen buffer
	char	clearDepth;		// Clear Depth buffer
} fbounbind_section;

// ******************************************************************

Section* instance_fboUnbind() {
	return new sFboUnbind();
}

sFboUnbind::sFboUnbind() {
	type = SectionType::FboUnbind;
}

bool sFboUnbind::load() {
	// script validation
	if (param.size() != 2) {
		LOG->Error("FboUnbind [%s]: 2 params are needed: clear the screen buffer, clear depth buffer", this->identifier.c_str());
		return false;
	}

	// load parameters
	clearScreen = (int)param[0];
	clearDepth = (int)param[1];
	return true;
}

void sFboUnbind::init() {
	
}

void sFboUnbind::exec() {
	// Unbind the fbo
	m_demo.fboManager.unbind(clearScreen, clearDepth);
}

void sFboUnbind::end() {
	
}

std::string sFboUnbind::debug() {
	return "[ fboUnbind id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
}
