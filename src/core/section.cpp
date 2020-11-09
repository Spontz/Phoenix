// Section.cpp
// Spontz Demogroup

#include "Section.h"
#include "main.h"

Section::Section()
	:
	m_demo(demokernel::GetInstance())
{
	
	type			= 0;
	identifier		= "No ID defined";
	DataSource		= "";
	startTime		= 0;
	endTime			= 0;
	duration		= 0;
	layer			= 0;
	enabled			= false;	// Disabled by default
	inited			= false;	// Not inited by default
	loaded			= false;	// Not loaded by default
	ended			= false;	// Not ended by default
	runTime			= 0;
	hasBlend		= 0;
	blendEquation	= 0;
	sfactor			= 0;
	dfactor			= 0;
	hasAlpha		= 0;
	alphaFunc		= 0;
	alpha1			= 0;
	alpha2			= 0;
}

Section::~Section()
{
	int i = 0;
	type = 0;
	identifier = "No ID defined";
	DataSource = "";
	startTime = 0;
	endTime = 0;
	duration = 0;
	layer = 0;
	enabled = false;	// Disabled by default
	inited = false;	// Not inited by default
	loaded = false;	// Not loaded by default
	ended = false;	// Not ended by default
	runTime = 0;
	hasBlend = 0;
	blendEquation = 0;
	sfactor = 0;
	dfactor = 0;
	hasAlpha = 0;
	alphaFunc = 0;
	alpha1 = 0;
	alpha2 = 0;

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


bool		Section::load() { return true; };
void		Section::init() {};
void		Section::exec() {};
void		Section::end() {};
std::string	Section::debug() { return {}; };
