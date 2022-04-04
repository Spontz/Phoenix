#include "main.h"

namespace Phoenix {

	class sFboBind final : public Section {
	public:
		sFboBind();
		~sFboBind();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		unsigned int	m_uiFboNum = 0;
	};

	// ******************************************************************

	Section* instance_fboBind()
	{
		return new sFboBind();
	}

	sFboBind::sFboBind()
	{
		type = SectionType::FboBind;
	}

	sFboBind::~sFboBind()
	{
	}

	bool sFboBind::load()
	{
		// script validation
		if (param.size() != 3) {
			Logger::error("FboBind [{}]: 3 params are needed: fbo to use, clear the screen buffer, clear depth buffer", identifier);
			return false;
		}

		// load parameters
		m_uiFboNum = static_cast<unsigned int>(param[0]);
		render_clearColor = static_cast<bool>(param[1]);
		render_clearDepth = static_cast<bool>(param[2]);

		if (m_uiFboNum >= m_demo.m_fboManager.fbo.size()) {
			Logger::error("FboBind [{}]: The fbo number {} cannot be accessed, check graphics.spo file", identifier, m_uiFboNum);
			return false;
		}

		return !DEMO_checkGLError();
	}

	void sFboBind::init()
	{

	}

	void sFboBind::exec()
	{
		// Enable the buffer in which we are going to paint
		m_demo.m_fboManager.bind(m_uiFboNum, render_clearColor, render_clearDepth);
	}

	void sFboBind::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Fbo: " << m_uiFboNum << std::endl;
		ss << "Fbo type: " << m_demo.m_fboManager.fbo[m_uiFboNum]->engineFormat << std::endl;
		ss << "Fbo attachments: " << m_demo.m_fboManager.fbo[m_uiFboNum]->numAttachments << std::endl;
		debugStatic = ss.str();
	}

	std::string sFboBind::debug()
	{
		return debugStatic;
	}
}