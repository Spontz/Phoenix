// section.cpp
// Spontz Demogroup

#include "section.h"
#include "main.h"

Section::Section() {
	int i			= 0;
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

void Section::EvalBlendingStart()
{
	if (this->hasBlend) {
		glEnable(GL_BLEND);
		glBlendFunc(this->sfactor, this->dfactor);
		if (this->blendEquation > 0)
			glBlendEquation(this->blendEquation);
	}
}

void Section::EvalBlendingEnd()
{
	if (this->hasBlend) {
		glDisable(GL_BLEND);
		if (this->blendEquation > 0)
			glBlendEquation(GL_FUNC_ADD);
	}
}