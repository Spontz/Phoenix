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
	bool	m_bClearScreen	= true;		// Clear Screen buffer
	bool	m_bClearDepth	= false;	// Clear Depth buffer
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
	m_bClearScreen = static_cast<bool>(param[0]);
	m_bClearDepth = static_cast<bool>(param[1]);
	return true;
}

void sFboUnbind::init() {
	
}

void sFboUnbind::exec() {
	// Unbind the fbo
	m_demo.fboManager.unbind(m_bClearScreen, m_bClearDepth);
}

void sFboUnbind::end() {
	
}

std::string sFboUnbind::debug() {
	std::stringstream ss;
	ss << "+ FboUnbind id: " << identifier << " layer: " << layer << std::endl;
	return ss.str();

}
