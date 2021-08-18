#include "main.h"

namespace Phoenix {

	class sDrawFbo final : public Section {
	public:
		sDrawFbo();
		~sDrawFbo();

	public:
		bool		load();
		void		init();
		void		exec();
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

	sDrawFbo::~sDrawFbo()
	{
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
		// Set render states
		render_clearColor = static_cast<bool>(param[2]);
		render_clearDepth = static_cast<bool>(param[3]);
		render_disableDepthTest = true;

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
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();
		{
			m_pFbo = m_demo.m_fboManager.fbo[m_uFboNum]; // We reload the Fbo, because it may be changed when resizing the screen
			m_demo.m_pRes->drawQuadFboFS(m_pFbo, m_uFboAttachment);
		}
		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
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