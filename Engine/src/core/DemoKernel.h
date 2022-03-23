// Demokernel.h
// Spontz Demogroup

#pragma once

namespace Phoenix { class DemoKernel; }

#include "core/Window.h"
#include "core/LayerStack.h"
#include "core/events/Event.h"
#include "core/events/DemoKernelEvent.h"

#include "core/layers/SectionLayer.h"
#include "core/layers/ImGuiLayer.h"

#include "drivers/GlDriver.h"
#include "drivers/NetDriver.h"

#include "core/renderer/Spline.h"
#include "core/Section.h"
#include "core/SectionManager.h"
#include "sections/Sections.h"

#include "core/renderer/VideoManager.h"
#include "core/renderer/TextureManager.h"
#include "core/renderer/FboManager.h"
#include "core/renderer/ModelManager.h"
#include "core/renderer/ShaderManager.h"
#include "core/renderer/LightManager.h"
#include "core/renderer/Font.h"
#include "core/renderer/Camera.h"
#include "core/renderer/CameraTypes.h"
#include "core/renderer/Light.h"
#include "core/utils/LoggerDeclarations.h"
#include "core/resource/Resource.h"

namespace Phoenix {


#define DEMO (&Phoenix::DemoKernel::getInstance())

	// Demo status
	enum DemoStatus : char
	{
		LOADING = 0,
		PLAY = 2,
		PAUSE = 4,
		REWIND = 8,
		FASTFORWARD = 16
	};

	class DemoKernel final
	{
	private:
		DemoKernel();
		~DemoKernel();

	public:
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Window& GetWindow() { return *m_Window; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		static DemoKernel& getInstance();
		static void release();
		static std::string getEngineVersion();
		static std::string getLibAssimpVersion();
		static std::string getLibDyadVersion();

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	public:
		// Resources Management
		void allocateResources();

		void getArguments(int32_t argc, char* argv[]);
		bool initDemo();
		void initNetwork();
		void Run();
		void Close();

		// Data folder operations
		bool checkDataFolder() const;
		std::string getDataFolder(std::string_view path) const;

		// Scripts (SPO files) management
		bool loadSpoConfig();
		void loadSpoFiles();
		bool loadScriptFromNetwork(std::string_view script);


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
		void calculateFPS(float frameRate);
		void initTimer();
		void processTimer();
		void pauseTimer();
		void checkDemoEnd();	// Checks if demo has been finished and should be restarted (or not)

		void initControlVars(); // Init control vars
		void initSectionQueues(); // Init Section Queues - TODO: DELETE THIS
		void reinitSectionQueues(); // ReInit Section Queues - TODO: DELETE THIS
		void processSectionQueues(); // Process Section Queues

	public:
		std::string m_dataFolder; // Path to "data" folder
		std::string m_demoName;

		// private: (hack: create getters)
		bool m_debug;
		float m_debugFontSize;
		LogLevel m_logLevel;
		bool m_slaveMode;

		// misc
		bool m_loop;
		bool m_sound;
		bool m_debug_drawGrid;
		bool m_debug_drawGridAxisX;
		bool m_debug_drawGridAxisY;
		bool m_debug_drawGridAxisZ;

		int32_t m_status; // Demo status (play, pause, loading, etc.)
		float m_demoStartTime; // first demo second
		float m_demoEndTime; // last demo second (0 = unlimited)

		// Managers
		SectionManager m_sectionManager;
		TextureManager m_textureManager;
		VideoManager m_videoManager;
		FboManager m_fboManager; // FBO Manager (FBO's free to be used by the demo maker)
		FboManager m_efxBloomFbo; // Bloom efx FBO Manager (internal FBO's used by the engine)
		FboManager m_efxAccumFbo; // Accumulation efx FBO Manager (internal engine FBO's)
		ModelManager m_modelManager;
		ShaderManager m_shaderManager;
		LightManager m_lightManager;

		Camera* m_pActiveCamera; // Current Active Camera
		Camera* m_pDefaultCamera; // Default Camera when no camera is defined
		Font* m_pText;

		// Resources
		Resource* m_pRes;

		// loading information
		int32_t m_iLoadedSections; // TODO: Delete

		// realtime information
		float m_demoRunTime; // seconds ellapsed since 0.0 <- demo init
		float m_beforeFrameTime; // time before render the actual frame
		float m_afterFrameTime; // time after render the actual frame
		float m_frameTime; // last frame time (used by sections)
		float m_realFrameTime; // last frame time (used by kernel)
		uint32_t m_uiFrameCount; // demo frame count since start

		// fps calculation
		uint32_t m_accumFrameCount; // frame count since last fps calculation
		float m_accumFrameTime; // time since last fps calculation
		float m_fps; // frames per second

		bool m_exitDemo; // exits demo at next loop if true

		int32_t m_mouseX, m_mouseY; // Mouse global coordinates
		int32_t m_mouseXVar, m_mouseYVar; // Mouse variation from the center of the screen

		// Window parameters
		bool m_overrideWindowConfigParams; // Override Window config file parameters
		int32_t m_windowPosX, m_windowPosY, m_windowWidth, m_windowHeight; // Window properties

		float m_fVar[MULTIPURPOSE_VARS]; // Multi-purpose variables, shared across sections
		float m_fBeat[MAX_BEATS]; // intensity of the current music track

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		SectionLayer* m_SectionLayer;
		LayerStack m_LayerStack;

	};

}

#include "core/utils/Logger.h"
