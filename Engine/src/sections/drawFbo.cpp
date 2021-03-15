#include "main.h"

namespace Phoenix {

	struct sDrawFbo : public Section {
	public:
		sDrawFbo();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		Fbo*		m_pFbo = nullptr;
		uint32_t	m_uFboNum = 0;
		uint32_t	m_uFboAttachment = 0;
		bool		m_bClearScreen = true;	// Clear Screen buffer
		bool		m_bClearDepth = true;	// Clear Depth buffer
	};

	// ******************************************************************

	Section* instance_drawFbo()
	{
		return new sDrawFbo();
	}

	sDrawFbo::sDrawFbo()
	{
		type = SectionType::DrawFbo;
	}

	bool sDrawFbo::load()
	{
		// script validation
		if (param.size() != 4) {
			Logger::error("DrawFbo [%s]: 4 params are needed: fbo to use and attachment, clear the screen buffer, clear depth buffer", identifier.c_str());
			return false;
		}

		// load parameters
		m_uFboNum = static_cast<uint32_t>(param[0]);
		m_uFboAttachment = static_cast<uint32_t>(param[1]);
		m_bClearScreen = static_cast<bool>(param[2]);
		m_bClearDepth = static_cast<bool>(param[3]);

		// Check for the right parameter values
		if (m_uFboNum >= FBO_BUFFERS) {
			Logger::error("DrawFbo [%s]: Invalid texture fbo number: %i", identifier.c_str(), m_uFboNum);
			return false;
		}

		m_pFbo = m_demo.m_fboManager.fbo[m_uFboNum];

		if (m_uFboAttachment >= static_cast<uint32_t>(m_pFbo->numAttachments)) {
			Logger::error("DrawFbo [%s]: Invalid texture fbo attachment: %i", identifier.c_str(), m_uFboAttachment);
			return false;
		}

		return true;
	}

	void sDrawFbo::init()
	{

	}

	void sDrawFbo::exec()
	{
		// Clear the screen and depth buffers depending of the parameters passed by the user
		if (m_bClearScreen) glClear(GL_COLOR_BUFFER_BIT);
		if (m_bClearDepth) glClear(GL_DEPTH_BUFFER_BIT);

		EvalBlendingStart();

		glDisable(GL_DEPTH_TEST);
		{
			m_pFbo = m_demo.m_fboManager.fbo[m_uFboNum]; // We reload the Fbo, because it may be changed when resizing the screen

			m_demo.m_pRes->Draw_QuadFBOFS(m_pFbo, m_uFboAttachment);
		}
		glEnable(GL_DEPTH_TEST);

		EvalBlendingEnd();
	}

	void sDrawFbo::end()
	{

	}

	void sDrawFbo::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Fbo: " << m_uFboNum << std::endl;
		ss << "Attachment: " << m_uFboAttachment << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawFbo::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}