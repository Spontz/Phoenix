// demoKernel.h
// Spontz Demogroup

#ifndef DEMOKERNEL_H
#define DEMOKERNEL_H

class demokernel;

#include "drivers/gldriver.h"

#include "core/spline.h"
#include "core/section.h"
#include "core/sectionmanager.h"
#include "sections/sections.h"

#include "core/video_manager.h"
#include "core/texturemanager.h"
#include "core/fbomanager.h"
#include "core/modelmanager.h"
#include "core/shadermanager.h"
#include "core/lightmanager.h"
#include "core/font.h"
#include "core/camera.h"
#include "core/light.h"
#include "core/utils/logger.h"

#include "core/resource/resource.h"

// ******************************************************************

#define DEMO (&demokernel::GetInstance())

// ******************************************************************

// Demo status
enum DemoStatus : char const {
	LOADING = 0,
	PLAY = 2,
	PAUSE = 4,
	REWIND = 8,
	FASTFORWARD = 16
};

// ******************************************************************
class demokernel final {
private:
	demokernel();
public:
	// ******************************************************************
	// VARIABLES
	std::string		dataFolder;	// Path to "data" folder
	char*			demoName;
	const bool		debug;
	const float		debug_fontSize;
	const LogLevel	m_logLevel;

	// misc
	int			loop;
	int			sound;

	int			state;				// state of the demo (play, pause, loading, etc.)
	float		demo_startTime;		// first demo second
	float		demo_endTime;		// last demo second (0 = unlimited)

	// Managers
	SectionManager	sectionManager;
	TextureManager	textureManager;
	VideoManager	m_videoManager_;
	FboManager		fboManager;					// Generic Fbo Manager (FBO's free to be used by the demo maker)
	FboManager		efxBloomFbo;				// Bloom efx Fbo Manager (internal FBO's used by the engine)
	FboManager		efxAccumFbo;				// Accumulation efx Fbo Manager (internal FBO's used by the engine)
	ModelManager	modelManager;
	ShaderManager	shaderManager;
	LightManager	lightManager;
	Camera			*camera;
	Font			*text;

	// Resources
	Resource		*res;

	// loading information
	int loadedSections;

	// realtime information
	float demo_runTime;							// seconds ellapsed since 0.0 <- demo init
	float beforeFrameTime;						// time before render the actual frame
	float afterFrameTime;						// time after render the actual frame
	float frameTime;							// last frame time (used by sections)
	float realFrameTime;						// last frame time (used by kernel)
	unsigned int frameCount;					// demo frame count since start

	// fps calculation
	unsigned int accumFrameCount;				// frame count since last fps calculation
	float accumFrameTime;						// time since last fps calculation
	float fps;									// frames per second

	char exitDemo;								// exits demo at next loop if true

	// Beat and beat detection parameters
	float beat;									// intensity of the current music beat
	float beat_ratio;
	float beat_fadeout;

	int mouseX, mouseY;							// Mouse global coordinates
	int mouseXvar, mouseYvar;					// Mouse variation rom the center of the screen

	// network slave mode
	const int slaveMode;						// 1 = network slave; 0 = standalone mode;

	// Multi-purpose variables, shared in for the sections
	float var[MULTIPURPOSE_VARS];

	// ******************************************************************
	// MEMBERS

	static demokernel& GetInstance();

	// Resources Management
	void allocateResources();

	void getArguments(int argc, char *argv[]);
	bool initDemo();
	void initNetwork();
	void mainLoop();
	void closeDemo();
	static std::string getEngineVersion();
	static std::string getLibAssimpVersion();
	static std::string getLibDyadVersion();

	// Data folder operations
	bool checkDataFolder();
	std::string getFolder(std::string path);

	// Scripts (SPO files) management
	bool load_config();
	void load_spos();
	bool load_scriptFromNetwork(std::string sScript);
	

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
};
// ******************************************************************

#endif