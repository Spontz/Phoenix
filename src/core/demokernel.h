// demoKernel.h
// Spontz Demogroup

#ifndef DEMOKERNEL_H
#define DEMOKERNEL_H

#include <iostream>
#include "core/section.h"
#include "core/sectionmanager.h"
#include "sections/sections.h"

#include "core/texturemanager.h"
#include "core/fbomanager.h"
#include "core/modelmanager.h"
#include "core/shadermanager.h"
#include "core/font.h"
#include "core/camera.h"

// ******************************************************************

#define SECTION_PARAMS		32
#define SECTION_STRINGS		32
#define SECTION_SPLINES		32
#define SECTION_MODIFIERS	32

#define RENDERING_BUFFERS	10
#define FBO_BUFFERS			25

// Warning! 'FBO_BUFFERS' should not be less than 25, if you want to set less than 25,
// please check the variable 'scriptCommand' contents in 'demokernel.cpp' file.

// ******************************************************************

// demo states
#define DEMO_LOADING		0
#define DEMO_PLAY			2
#define DEMO_PAUSE			4
#define DEMO_REWIND			8
#define DEMO_FASTFORWARD	16

// ******************************************************************

#define DEMO demokernel::getInstance()

// ******************************************************************

typedef struct {
	char *name;
	int code;
} glTable_t;

// ******************************************************************

class demokernel {

public:
	// ******************************************************************
	// VARIABLES
	char* demoDir;
	char* demoName;
	int debug;
	// misc
	int record;
	float recordFps;
	int compressTga;
	int loop;
	int sound;
	int bench;

	int state;									// state of the demo (play, pause, loading, etc.)
	float startTime;							// first demo second
	float endTime;								// last demo second (0 = unlimited)

	// Managers
	SectionManager	sectionManager;
	TextureManager	textureManager;
	FboManager		fboManager;
	ModelManager	modelManager;
	ShaderManager	shaderManager;
	Camera *camera;		//TODO: Implement a camera manager... does it makes sense¿?
	Font	*text;

	// loading information
	//int numSections;
	//int numReadySections;
	int loadedSections;

	// realtime information
	float runTime;								// seconds ellapsed since 0.0 <- demo init, not SDL init
	float beforeFrameTime;						// time before render the actual frame
	float afterFrameTime;						// time after render the actual frame
	float frameTime;							// last frame time (used by sections)
	float realFrameTime;						// last frame time (used by kernel)
	unsigned int frameCount;					// demo frame count since start

	// fps calculation
	unsigned int accumFrameCount;				// frame count since last fps calculation
	float accumFrameTime;						// time since last fps calculation
	float fps;									// frames per second

	int keys[512];								// keyboard control (SDL 2.0 handles 512 diferent scan codes)
	char mouseButton;							// left mouse button status
	char exitDemo;								// exits demo at next loop if true

	//camera_t *camera;							// the actual camera (set by genCamera)
	int rtt, backup;							// render to texture shared texture index (RGB8 format)

	// Beat and beat detection parameters
	float beat;									// intensity of the current music beat
	float beat_ratio;
	float beat_fadeout;

	int mouseX, mouseY;							// Mouse global coordinates
	int mouseXvar, mouseYvar;					// Mouse variation rom the center of the screen
	char mouseButtons;							// Mouse buttons state

	// network slave mode
	int slaveMode; // 1 = network slave; 0 = standalone mode;

	// Drawing states
	char drawSound;								// Draw sound render
	char drawTiming;							// Draw time of the demo
	char drawFps;								// Draw FPS's of the demo

	// engine state variables
	//matrix_t	m_VariableMatrix[MAX_SVE_VARIABLE_ID + 1];


	// ******************************************************************
	// MEMBERS

	static demokernel* getInstance();
	demokernel();
	void getArguments(int argc, char *argv[]);
	void initDemo();
	void mainLoop();
	void closeDemo();

	// Scripts (SPO files) management
	void load_spos();

	// Demo control
	void playDemo();
	void pauseDemo();
	void restartDemo();
	void rewindDemo();
	void fastforwardDemo();

	
private:

	static demokernel* m_pThis;
	
	void doExec();

	// Timing controls
	void calculateFPS(float frameRate); 
	void initTimer();
	void processTimer();
	void pauseTimer();
	

	void initControlVars();			//Init control vars
	void initSectionQueues();		// Init Section Queues
	void reInitSectionQueues();		// ReInit Section Queues
	void processSectionQueues();	// Process Section Queues


	// Scripts (SPO files) management
	void load_spo(string sFile);
	string load_file(string sFile);
	void load_scriptData(string sScript, string sFile);
	int getSectionByName(char *name);
	int getCodeByName(char *name, glTable_t *table, int size);
	int getBlendCodeByName(char *name);
	int getBlendEquationCodeByName(char *name);
	int getAlphaCodeByName(char *name);
	int getRenderModeByName(char *name);
};
// ******************************************************************

#endif