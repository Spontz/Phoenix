// Section.cpp
// Spontz Demogroup

#include "Section.h"
#include "main.h"

namespace Phoenix {

	Section::Section()
		:
		m_demo(demokernel::GetInstance()),
		type(SectionType::NOT_FOUND),
		identifier("ID not defined"),
		DataSource(""),
		startTime(0),
		endTime(0),
		duration(0),
		layer(0),
		enabled(false),
		inited(false),
		loaded(false),
		ended(false),
		runTime(0),
		hasBlend(0),
		blendEquation(0),
		sfactor(0),
		dfactor(0),
		debugStatic(""),
		render_drawWireframe(false),
		render_clearColor(false),
		render_clearDepth(false),
		render_disableDepthTest(false),
		render_disableDepthMask(false)		
	{
	}

	Section::~Section()
	{
		param.clear();
		strings.clear();
		uniform.clear();
		spline.clear();
	}

	void Section::EvalBlendingStart()
	{
		if (hasBlend) {
			glEnable(GL_BLEND);
			glBlendFunc(sfactor, dfactor);
			if (blendEquation > 0)
				glBlendEquation(blendEquation);
		}
	}

	void Section::EvalBlendingEnd()
	{
		if (hasBlend) {
			glDisable(GL_BLEND);
			if (blendEquation > 0)
				glBlendEquation(GL_FUNC_ADD);
		}
	}

	void Section::setRenderStatesStart()
	{
		// Clear color
		if (render_clearColor)	
			glClear(GL_COLOR_BUFFER_BIT);
		// Clear depth
		if (render_clearDepth)	
			glClear(GL_DEPTH_BUFFER_BIT);
		// Wireframe mode
		if(render_drawWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// Disable depth test
		if (render_disableDepthTest)
			glDisable(GL_DEPTH_TEST);
		// Disable Depth Mask
		if (render_disableDepthMask)
			glDepthMask(GL_FALSE);
	}

	void Section::setRenderStatesEnd()
	{
		// Restore fill mode (defualt mode)
		if (render_drawWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Restore Depth Test (default mode: enabled)
		if (render_disableDepthTest)
			glEnable(GL_DEPTH_TEST);
		// Restore Depth Mask writing (default mode: enabled)
		if (render_disableDepthMask)
			glDepthMask(GL_TRUE);
	}


	bool		Section::load() { return true; };
	void		Section::init() {};
	void		Section::exec() {};
	void		Section::end() {};
	void		Section::loadDebugStatic() {};
	std::string	Section::debug() { return {}; };
}