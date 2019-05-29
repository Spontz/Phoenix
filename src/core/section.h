// section.h
// Spontz Demogroup

#ifndef SECTION_H
#define SECTION_H
#include <string>
#include <vector>
#include "spline.h"
using namespace std;

#define SECTION_PARAMS		32
#define SECTION_STRINGS		32
#define SECTION_SPLINES		32
#define SECTION_MODIFIERS	32

typedef struct {
	int spline;
	int param;
	int mode;
} Modifier;

class Section {
public:
	int					type;
	// section data
	string				identifier;							// Unique section identifier generated by the demo editor
	string				DataSource;							// origin of this section (for error messages)
	string				type_str;							// Section type (in string format)
	float				startTime;							// section start time
	float				endTime;							// section end time
	float				duration;							// total section seconds
	int					layer;								// layer where sections run (priority)
	int					enabled;							// 1: Section enabled; 0: Section disabled
	int					inited;								// 1: Section inited; 0: Section needs to be inited
	int					loaded;								// 1: Section loaded; 0: Section not loaded
	int					ended;								// 1: Section ended; 0: Section not ended
	float				runTime;							// seconds elapsed since section start
	int					hasBlend;							// has blending function parameters?
	int					blendEquation;						// Blend Equation
	int					sfactor;							// default source blending factor
	int					dfactor;							// default dest blending factor
	int					hasAlpha;							// has alpha function parameters?
	int					alphaFunc;							// alpha function
	float				alpha1;								// TODO: alpha reference value (hack: no se sabe bien que es esto), renombrar estas variables y documentar aqui
	float				alpha2;								// TODO: alpha reference value (hack: no se sabe bien que es esto), renombrar estas variables y documentar aqui
	int					paramNum;							// number of parameters
	float				param[SECTION_PARAMS];				// float parameters
	int					stringNum;							// number of strings
	string				strings[SECTION_STRINGS];			// string parameters
	int					splineNum;							// number of splines
	vector <Spline*>	spline;								// Splines
	//float		splineDuration[SECTION_SPLINES];	// spline duration in seconds // TODO: To delete once is working
	void*		vars;								// pointer to section local variables


	// Members
	Section();
	virtual ~Section() {};

	// Blending management
	void EvalBlendingStart();
	void EvalBlendingEnd();

	// Splines management
	void loadSplines();
	// ******************************************************************

	virtual bool load() { return true; };	// load data from disk
	virtual void init()		{};	// set up local variables before execution
	virtual void exec()		{};	// render a new frame
	virtual void end ()		{};	// destroy allocated memory
			
private:

};

#endif