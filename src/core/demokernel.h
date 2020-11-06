// demoKernel.h
// Spontz Demogroup

#pragma once

class demokernel;

#include "drivers/gldriver.h"

#include "core/spline.h"
#include "core/section.h"
#include "core/section_manager.h"
#include "sections/sections.h"

#include "core/video_manager.h"
#include "core/texture_manager.h"
#include "core/fbo_manager.h"
#include "core/model_manager.h"
#include "core/shader_manager.h"
#include "core/light_manager.h"
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
	std::string		m_dataFolder;	// Path to "data" folder
	char*			m_demoName;
	const bool		m_debug;
	const float		m_debug_fontSize;
	const LogLevel	m_logLevel;

	// misc // TODO: should be boolean values
	int			m_loop;
	int			m_sound;

	int			m_status;			// Demo status (play, pause, loading, etc.) - TODO: change int to struct
	float		m_demoStartTime;	// first demo second
	float		m_demoEndTime;		// last demo second (0 = unlimited)

	// Managers
	SectionManager	m_sectionManager;
	TextureManager	m_textureManager;
	VideoManager	m_videoManager;
	FboManager		m_fboManager;				// Generic Fbo Manager (FBO's free to be used by the demo maker)
	FboManager		m_efxBloomFbo;				// Bloom efx Fbo Manager (internal FBO's used by the engine)
	FboManager		m_efxAccumFbo;				// Accumulation efx Fbo Manager (internal FBO's used by the engine)
	ModelManager	m_modelManager;
	ShaderManager	m_shaderManager;
	LightManager	m_lightManager;
	Camera			*m_pCamera;
	Font			*m_pText;

	// Resources
	Resource		*m_pRes;

	// loading information
	int m_iLoadedSections;

	// realtime information
	float m_demoRunTime;						// seconds ellapsed since 0.0 <- demo init
	float m_beforeFrameTime;					// time before render the actual frame
	float m_afterFrameTime;						// time after render the actual frame
	float m_frameTime;							// last frame time (used by sections)
	float m_realFrameTime;						// last frame time (used by kernel)
	unsigned int m_uiFrameCount;				// demo frame count since start

	// fps calculation
	unsigned int m_accumFrameCount;				// frame count since last fps calculation
	float m_accumFrameTime;						// time since last fps calculation
	float m_fps;								// frames per second

	bool m_exitDemo;							// exits demo at next loop if true

	float m_beat;								// intensity of the current music beat

	int m_mouseX, m_mouseY;						// Mouse global coordinates
	int m_mouseXvar, m_mouseYvar;				// Mouse variation from the center of the screen

	// network slave mode
	const int m_slaveMode;						// 1 = network slave; 0 = standalone mode;

	// Multi-purpose variables, shared in for the sections
	float m_var[MULTIPURPOSE_VARS];

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
