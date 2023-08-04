// Demokernel.h
// Spontz Demogroup

#pragma once

namespace Phoenix { class DemoKernel; }

#include "core/Window.h"
#include "core/LayerStack.h"
#include "core/events/Event.h"
#include "core/events/DemoKernelEvent.h"

#include "core/input/Input.h"

#include "core/layers/SectionLayer.h"
#include "core/layers/ImGuiLayer.h"

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
#include "core/renderer/CameraManager.h"
#include "core/renderer/Font.h"
#include "core/renderer/Camera.h"
#include "core/renderer/CameraTypes.h"
#include "core/renderer/Light.h"
#include "core/utils/LoggerDeclarations.h"
#include "core/resource/Resource.h"

namespace Phoenix {


#define DEMO (&Phoenix::DemoKernel::getInstance())
#define DEMO_checkGLError() DEMO->m_Window->checkError_(__FILE__, __LINE__) 

	// Demo status
	enum DemoStatus : int8_t
	{
		LOADING = 0,
		PLAY = 2,
		PAUSE = 4,
		REWIND = 8,
		FASTFORWARD = 16
	};

	// Demo special events
	enum DemoSpecialEvent : int8_t
	{
		NORMAL = 0,
		SEEKTIME = 2,		// Seek time of the demo
		ONEFRAMEONLY = 4	// Rewind/FFW one frame only
	};

	class DemoKernel final
	{

		struct KeyStatus final {
			bool leftCtrl = false;
			bool leftShift = false;
			bool rightCtrl = false;
			bool rightShift = false;
		};

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
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnKeyReleased(KeyReleasedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMove(WindowMoveEvent& e);
		void OnProcessInput(float timeStep);

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

		// ImGUI management
		bool ImGuiWantCaptureMouse();
		void ImGuiAddLogMessage(std::string_view message);

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
	
		// Timing controls
		void calculateFPS(float frameRate);
		void initTimer();
		void processTimer();
		void pauseTimer();
		void checkDemoEnd();	// Checks if demo has been finished and should be restarted (or not)
		void initExecTimer();	// Init timer of execution time
		void processExecTimer();// Process timer of execution time


		void ProcessAndExecuteSectionsLayer();	// Process and Execute (render) the layer with all the sections
		void ProcessAndExecuteLayers();			// Process and Execute (render) the other layers that may be added by other apps
		void ProcessAndExecuteImGUILayer();		// Process and Execute (render) the layer with the ImGUI debug info

		void InitControlVars(); // Init control vars

	public:
		std::string m_dataFolder; // Path to "data" folder
		std::string m_demoName;

		// private: (hack: create getters)
		bool m_debug;
		LogLevel m_logLevel;
		bool m_slaveMode;

		// misc
		bool m_loop;
		bool m_sound;
		float m_debugFontSize;
		bool m_debugEnableGrid;
		bool m_debugDrawGridAxisX;
		bool m_debugDrawGridAxisY;
		bool m_debugDrawGridAxisZ;

		int8_t m_status;		// Demo status (play, pause, loading, etc.)
		int8_t m_specialEvent;	// Demo special events (seek time)
		float m_demoStartTime;	// first demo second
		float m_demoEndTime;	// last demo second (0 = unlimited)

		// Managers
		SectionManager m_sectionManager;
		TextureManager m_textureManager;
		VideoManager m_videoManager;
		FboManager m_fboManager;		// FBO Manager (FBO's free to be used by the demo maker)
		FboManager m_efxBloomFbo;		// Bloom efx FBO Manager (internal FBO's used by the engine)
		FboManager m_efxAccumFbo;		// Accumulation efx FBO Manager (internal engine FBO's)
		ModelManager m_modelManager;
		ShaderManager m_shaderManager;
		LightManager m_lightManager;
		CameraManager m_cameraManager;

		Font* m_pText;

		// Resources
		Resource* m_pRes;

		// Frame time stats
		float m_demoRunTime;		// seconds ellapsed since 0.0 <- demo init
		float m_beforeFrameTime;	// time before render the actual frame
		float m_afterFrameTime;		// time after render the actual frame
		float m_frameTime;			// last frame time (used by sections). If the demo is paused, this frameTime will be 0
		float m_realFrameTime;		// last frame time (used by kernel). Even if the demo is paused, the frameTime will be calculated
		uint32_t m_uiFrameCount;	// demo frame count since start
		
		// Execution time stats
		double m_beforeExecTime;	// Time before execution of sections
		double m_afterExecTime;		// Time after execution of sections
		double m_execTime;			// Execution time of sections

		// fps calculation
		uint32_t m_accumFrameCount;	// frame count since last fps calculation
		float m_accumFrameTime;		// time since last fps calculation
		float m_fps;				// frames per second

		bool m_exitDemo;			// exits demo at next loop if true

		// Window parameters
		bool m_overrideWindowConfigParams; // Override Window config file parameters
		int32_t m_windowPosX, m_windowPosY;
		int32_t m_windowWidth, m_windowHeight; // Window properties

		float m_fVar[MULTIPURPOSE_VARS]; // Multi-purpose variables, shared across sections
		float m_fBeat[MAX_BEATS]; // intensity of the current music track

		std::unique_ptr<Window> m_Window;

	private:
		
		ImGuiLayer* m_ImGuiLayer;
		SectionLayer* m_SectionLayer;
		LayerStack m_LayerStack;
		
		// Keyboard status
		KeyStatus keyStatus;

		bool m_WindowResizing;
	};

}

#include "core/utils/Logger.h"
#include "core/utils/Utils.h"
