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
	int		m_iFboNum			= 0;
	int		m_iFboAttachment	= 0;
	bool	m_bClearScreen		= true;	// Clear Screen buffer
	bool	m_bClearDepth		= true;	// Clear Depth buffer
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
		Logger::error("DrawFbo [%s]: 4 params are needed: fbo to use and attachment, clear the screen buffer, clear depth buffer", identifier.c_str());
		return false;
	}

	// load parameters
	m_iFboNum = static_cast<int>(param[0]);
	m_iFboAttachment = static_cast<int>(param[1]);
	m_bClearScreen = static_cast<bool>(param[2]);
	m_bClearDepth = static_cast<bool>(param[3]);

	// Check for the right parameter values
	if ((m_iFboNum < 0) || (m_iFboNum > (float)FBO_BUFFERS)) {
		Logger::error("DrawFbo [%s]: Invalid texture fbo number: %i", identifier.c_str(), m_iFboNum);
		return false;
	}

	if ((m_iFboAttachment < 0) || (m_iFboAttachment > (float)GLDRV_MAX_COLOR_ATTACHMENTS)) {
		Logger::error("DrawFbo [%s]: Invalid texture fbo attachment: %i", identifier.c_str(), m_iFboAttachment);
		return false;
	}

	return true;
}

void sDrawFbo::init() {
	
}

void sDrawFbo::exec() {
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (m_bClearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (m_bClearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	EvalBlendingStart();

	glDisable(GL_DEPTH_TEST);
	{
		m_demo.m_pRes->Draw_QuadFBOFS(m_iFboNum, m_iFboAttachment);
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawFbo::end() {
	
}

std::string sDrawFbo::debug() {
	std::stringstream ss;
	ss << "+ DrawFbo id: " << identifier << " layer: " << layer << std::endl;
	ss << "  fbo: " << m_iFboNum << std::endl;
	return ss.str();
}
