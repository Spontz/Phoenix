// demoKernel.h
// Spontz Demogroup

#ifndef DEMOKERNEL_H
#define DEMOKERNEL_H

#include <iostream>
#include "core/spline.h"
#include "core/section.h"
#include "core/sectionmanager.h"
#include "sections/sections.h"

#include "core/videomanager.h"
#include "core/texturemanager.h"
#include "core/fbomanager.h"
#include "core/modelmanager.h"
#include "core/shadermanager.h"
#include "core/lightmanager.h"
#include "core/font.h"
#include "core/camera.h"
#include "core/light.h"

// ******************************************************************

#define SECTION_PARAMS			32
#define SECTION_STRINGS			32
#define SECTION_SPLINES			32
#define SECTION_MODIFIERS		32

#define RENDERING_BUFFERS		10
#define FBO_BUFFERS				25
#define EFXBLOOM_FBO_BUFFERS	2
#define EFXACCUM_FBO_BUFFERS	2
#define MULTIPURPOSE_VARS		10

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

#define DEMO (&demokernel::GetInstance())

// ******************************************************************

typedef struct {
	char *name;
	int code;
} glTable_t;

// ******************************************************************

class demokernel {
private:
	demokernel();
public:
	// ******************************************************************
	// VARIABLES
	string dataFolder;	// Path to "data" folder
	char* demoName;
	int debug;
	int log_detail;

	// misc
	int loop;
	int sound;

	int state;									// state of the demo (play, pause, loading, etc.)
	float startTime;							// first demo second
	float endTime;								// last demo second (0 = unlimited)

	// Managers
	SectionManager	sectionManager;
	TextureManager	textureManager;
	VideoManager	videoManager;
	FboManager		fboManager;					// Generic Fbo Manager (FBO's free to be used by the demo maker)
	FboManager		efxBloomFbo;				// Bloom efx Fbo Manager (internal FBO's used by the engine)
	FboManager		efxAccumFbo;				// Accumulation efx Fbo Manager (internal FBO's used by the engine)
	ModelManager	modelManager;
	ShaderManager	shaderManager;
	LightManager	lightManager;
	Camera			*camera;
	Font			*text;

	// loading information
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

	int rtt, backup;							// render to texture shared texture index (RGB8 format)

	// Beat and beat detection parameters
	float beat;									// intensity of the current music beat
	float beat_ratio;
	float beat_fadeout;

	int mouseX, mouseY;							// Mouse global coordinates
	int mouseXvar, mouseYvar;					// Mouse variation rom the center of the screen
	char mouseButtons;							// Mouse buttons state

	// network slave mode
	int slaveMode;								// 1 = network slave; 0 = standalone mode;

	// Drawing states
	char drawTiming;							// Draw time of the demo
	char drawFps;								// Draw FPS's of the demo
	char drawSceneInfo;							// Draw debug Info from teh sections

	// Drawing
	char drawFbo;								// Draw FBO's
	char drawFboAttachment;						// FBO Attachment to draw

	// Multi-purpose variables, shared in for the sections
	float var[MULTIPURPOSE_VARS];

	// ******************************************************************
	// MEMBERS

	static demokernel& GetInstance();

	void getArguments(int argc, char *argv[]);
	void initDemo();
	void mainLoop();
	void closeDemo();

	// Data folder operations
	bool checkDataFolder();
	string getFolder(string path);

	// Scripts (SPO files) management
	bool load_config();
	void load_spos();
	bool load_scriptFromNetwork(string sScript);
	

	// Demo control
	void playDemo();
	void pauseDemo();
	void restartDemo();
	void rewindDemo();
	void fastforwardDemo();

	void setStartTime(float theTime);
	void setCurrentTime(float theTime);
	void setEndTime(float theTime);

	
private:

	static demokernel* m_pThis;
	
	void doExec();

	// Timing controls
	void calculateFPS(float const frameRate); 
	void initTimer();
	void processTimer();
	void pauseTimer();
	

	void initControlVars();			//Init control vars
	void initSectionQueues();		// Init Section Queues
	void reInitSectionQueues();		// ReInit Section Queues
	void processSectionQueues();	// Process Section Queues

	// Scripts (SPO files) management
	void load_spo(string sFile);
	int load_scriptData(string sScript, string sFile);
	static std::string load_ascii_file(std::string const& sFile);
	int getCodeByName(char *name, glTable_t *table, int size);
	int getBlendCodeByName(char *name);
	int getBlendEquationCodeByName(char *name);
	int getAlphaCodeByName(char *name);
};
// ******************************************************************

#endif