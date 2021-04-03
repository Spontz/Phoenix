// Demokernel.h
// Spontz Demogroup

#pragma once

namespace Phoenix { class demokernel; }

#include "drivers/gldriver.h"

#include "core/renderer/Spline.h"
#include "core/Section.h"
#include "core/SectionManager.h"
#include "sections/sections.h"

#include "core/renderer/VideoManager.h"
#include "core/renderer/TextureManager.h"
#include "core/renderer/FboManager.h"
#include "core/renderer/ModelManager.h"
#include "core/renderer/ShaderManager.h"
#include "core/renderer/LightManager.h"
#include "core/renderer/Font.h"
#include "core/renderer/Camera.h"
#include "core/renderer/Light.h"
#include "core/utils/logger.h"

#include "core/resource/resource.h"

namespace Phoenix {


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
		std::string		m_dataFolder;	// Path to "data" folder
		std::string		m_demoName;

	// private: (hack: create getters)
		bool		m_debug;
		float		m_debug_fontSize;
		LogLevel	m_logLevel;
		bool		m_slaveMode;

		// misc
		bool			m_loop;
		bool			m_sound;
		bool			m_debug_drawGrid;

		int				m_status;			// Demo status (play, pause, loading, etc.)
		float			m_demoStartTime;	// first demo second
		float			m_demoEndTime;		// last demo second (0 = unlimited)

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
		Camera* m_pCamera;
		Font* m_pText;

		// Resources
		Resource* m_pRes;

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

		int m_mouseX, m_mouseY;						// Mouse global coordinates
		int m_mouseXvar, m_mouseYvar;				// Mouse variation from the center of the screen

		// Window parameters
		bool m_overrideWindowConfigParams;	// Override Window config file parameters
		int m_windowPosX, m_windowPosY, m_windowWidth, m_windowHeight;	// Window properties


		float m_fVar[MULTIPURPOSE_VARS];			// Multi-purpose variables, shared across sections
		float m_fBeat[MAX_BEATS];				// intensity of the current music track

		// ******************************************************************
		// MEMBERS

		static demokernel& GetInstance();

		// Resources Management
		void allocateResources();

		void getArguments(int argc, char* argv[]);
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
		bool loadSpoConfig();
		void loadSpoFiles();
		bool loadScriptFromNetwork(std::string sScript);


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
}