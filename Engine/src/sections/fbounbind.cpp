#include "main.h"

namespace Phoenix {
	struct sFboUnbind : public Section {
	public:
		sFboUnbind();
		bool		load();
		void		init();
		void		exec();
		void		destroy();
		void		loadDebugStatic();
		std::string debug();

	private:
	} fbounbind_section;

	// ******************************************************************

	Section* instance_fboUnbind()
	{
		return new sFboUnbind();
	}

	sFboUnbind::sFboUnbind()
	{
		type = SectionType::FboUnbind;
	}

	bool sFboUnbind::load()
	{
		// script validation
		if (param.size() != 2) {
			Logger::error("FboUnbind [%s]: 2 params are needed: clear the screen buffer, clear depth buffer", this->identifier.c_str());
			return false;
		}

		// load parameters
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		return true;
	}

	void sFboUnbind::init()
	{

	}

	void sFboUnbind::exec()
	{
		// Unbind the fbo
		m_demo.m_fboManager.unbind(render_clearColor, render_clearDepth);
	}

	void sFboUnbind::destroy()
	{

	}

	void sFboUnbind::loadDebugStatic()
	{
	}

	std::string sFboUnbind::debug()
	{
		return debugStatic;

	}
}