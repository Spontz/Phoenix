#include "main.h"

namespace Phoenix {

	struct sFboBind : public Section {
	public:
		sFboBind();
		bool		load();
		void		init();
		void		exec();
		void		end();
		void		loadDebugStatic();
		std::string debug();

	private:
		unsigned int	m_uiFboNum = 0;
		bool			m_bClearScreen = true;		// Clear Screen buffer
		bool			m_bClearDepth = false;	// Clear Depth buffer
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

	bool sFboBind::load()
	{
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

	void sFboBind::init()
	{

	}

	void sFboBind::exec()
	{

		// Enable the buffer in which we are going to paint
		m_demo.m_fboManager.bind(m_uiFboNum, m_bClearScreen, m_bClearDepth);
	}

	void sFboBind::end()
	{

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