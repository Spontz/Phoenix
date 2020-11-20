// Section.cpp
// Spontz Demogroup

#include "Section.h"
#include "main.h"

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
	dfactor(0)
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


bool		Section::load() { return true; };
void		Section::init() {};
void		Section::exec() {};
void		Section::end() {};
std::string	Section::debug() { return {}; };
