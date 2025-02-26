#include "main.h"

namespace Phoenix {
	class sFboUnbind final : public Section {
	public:
		sFboUnbind();
		~sFboUnbind();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
	};

	// ******************************************************************

	Section* instance_fboUnbind()
	{
		return new sFboUnbind();
	}

	sFboUnbind::sFboUnbind()
	{
		type = SectionType::FboUnbind;
	}

	sFboUnbind::~sFboUnbind()
	{
	}

	bool sFboUnbind::load()
	{
		// script validation
		if (param.size() != 2) {
			Logger::error("FboUnbind [{}]: 2 params (clear the screen buffer and clear depth buffer) are needed", identifier);
			return false;
		}

		// load parameters
		render_clearColor = static_cast<bool>(param[0]);
		render_clearDepth = static_cast<bool>(param[1]);
		
		return !DEMO_checkGLError();
	}

	void sFboUnbind::init()
	{

	}

	void sFboUnbind::exec()
	{
		// Unbind the fbo
		m_demo.m_fboManager.unbind(render_clearColor, render_clearDepth);
	}

	void sFboUnbind::loadDebugStatic()
	{
	}

	std::string sFboUnbind::debug()
	{
		return debugStatic;

	}
}