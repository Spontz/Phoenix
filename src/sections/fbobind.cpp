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
	unsigned int	m_uiFboNum		= 0;
	bool			m_bClearScreen	= true;		// Clear Screen buffer
	bool			m_bClearDepth	= false;	// Clear Depth buffer
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
		Logger::error("FboBind [%s]: 3 params are needed: fbo to use, clear the screen buffer, clear depth buffer", identifier.c_str());
		return false;
	}

	// load parameters
	m_uiFboNum = static_cast<unsigned int>(param[0]);
	m_bClearScreen = static_cast<bool>(param[1]);
	m_bClearDepth = static_cast<bool>(param[2]);

	if (m_uiFboNum >= m_demo.m_fboManager.fbo.size()) {
		Logger::error("FboBind [%s]: The fbo number %i cannot be accessed, check graphics.spo file", identifier.c_str(), m_uiFboNum);
		return false;
	}

	return true;
}

void sFboBind::init() {
	
}

void sFboBind::exec() {

	// Enable the buffer in which we are going to paint
	m_demo.m_fboManager.bind(m_uiFboNum, m_bClearScreen, m_bClearDepth);
}

void sFboBind::end() {
	
}

std::string sFboBind::debug() {
	std::stringstream ss;
	ss << "+ FboBind id: " << identifier << " layer: " << layer << std::endl;
	ss << "  fbo: " << m_uiFboNum << std::endl;
	return ss.str();
}
