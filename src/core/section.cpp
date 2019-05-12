// section.cpp
// Spontz Demogroup

#include "section.h"


Section::Section() {
	int i = 0;
	type = 0;
	identifier = "No ID defined";
	DataSource = "";
	startTime	= 0;
	endTime		= 0;
	duration	= 0;
	layer		= 0;
	enabled		= 0;	// Disabled by default
	inited		= 0;	// Not inited by default
	loaded		= 0;	// Not loaded by default
	ended		= 0;	// Not ended by default
	runTime		= 0;
	hasBlend	= 0;
	sfactor		= 0;
	dfactor		= 0;
	hasAlpha	= 0;
	alphaFunc	= 0;
	alpha1		= 0;
	alpha2		= 0;
	paramNum	= 0;
	for (i = 0; i<SECTION_PARAMS; i++)
		param[i]	= 0;
	stringNum	= 0;
	for (i=0; i<SECTION_STRINGS; i++)
		strings[i] = "";
	splineNum	= 0;
	// TODO: Add splines support
	//Motion*		splines[SECTION_SPLINES];			// spline pointers
	for (i = 0; i < SECTION_SPLINES; i++) {
		splineFiles[i] = "";
		splineDuration[i] = 0;
	}
	
	
}

