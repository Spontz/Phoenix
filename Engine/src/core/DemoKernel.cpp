// Demokernel.cpp
// Spontz Demogroup

#include "main.h"
#include "core/scripting/SpoReader.h"
#include "core/drivers/glDriver.h"
#include "core/drivers/BassDriver.h"
#include "core/drivers/NetDriver.h"
#include "core/resource/Resource.h"

#ifdef PROFILE_PHOENIX
#include "debug/Instrumentor.h"
#endif

#include <io.h>
#include <iostream>

namespace Phoenix {

	DemoKernel* kpDemoKernel = nullptr;

	void DemoKernel::PushLayer(Layer* layer)
	{
		PX_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void DemoKernel::PushOverlay(Layer* layer)
	{
		PX_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void DemoKernel::OnEvent(Event& e)
	{
		PX_PROFILE_FUNCTION();

		// Send to the dispatcher the events that DemoKernel can handle
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(PX_BIND_EVENT_FN(DemoKernel::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(PX_BIND_EVENT_FN(DemoKernel::OnWindowResize));
		dispatcher.Dispatch<KeyPressedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnKeyReleased));
		dispatcher.Dispatch<MouseButtonPressedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnMouseButtonPressed));
		
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}
	
	bool DemoKernel::OnKeyPressed(KeyPressedEvent& e)
	{
		uint16_t key = e.GetKeyCode();
		bool EventHandled = false;

		if (key == Key::EXIT)
		{
			m_exitDemo = true;
			EventHandled = true;
		}
		if (m_debug) {
			EventHandled = true;
			switch (key) {
			case Key::FASTFORWARD:
				fastforwardDemo();
				break;
			case Key::REWIND:
				rewindDemo();
				break;
			case Key::TIME:
				Logger::info(LogLevel::high, "Demo time: {:.4f}", m_demoRunTime);
				break;
			case Key::PLAY_PAUSE:
				if (m_status == DemoStatus::PLAY)
					pauseDemo();
				else
					playDemo();
				break;
			case Key::RESTART:
				restartDemo();
				break;
			/*
			case Key::SHOWLOG:
				m_ImGuiLayer->show_log = !m_ImGuiLayer->show_log;
				break;
			case Key::SHOWINFO:
				m_ImGuiLayer->show_info = !m_ImGuiLayer->show_info;
				break;
			case Key::SHOWVERSION:
				m_ImGuiLayer->show_version = !m_ImGuiLayer->show_version;
				break;
			case Key::SHOWFPSHIST:
				m_ImGuiLayer->show_fpsHistogram = !m_ImGuiLayer->show_fpsHistogram;
				break;
			case Key::SHOWFBO:
				m_ImGuiLayer->show_fbo = !m_ImGuiLayer->show_fbo;
				break;
			case Key::CHANGEATTACH:
				m_ImGuiLayer->m_numFboAttachmentToDraw++;
				if (m_ImGuiLayer->m_numFboAttachmentToDraw >= FBO_MAX_COLOR_ATTACHMENTS)
					m_ImGuiLayer->m_numFboAttachmentToDraw = 0;
				break;
			case Key::SHOWSECTIONINFO:
				//GLDRV->guiDrawSections();
				break;
			case Key::SHOWSOUND:
				//GLDRV->guiDrawSound();
				break;
			case Key::SHOWGRIDPANEL:
				//GLDRV->guiDrawGridPanel();
				break;
			case Key::SHOWHELP:
				//GLDRV->guiDrawHelpPanel();
				break;
				*/
			case Key::CAM_CAPTURE:
				if (m_pActiveCamera->capturePos()) {
					Logger::sendEditor("Camera position saved!");
				}
				else {
					Logger::error("Camera file was not saved");
				}
				break;
			case Key::CAM_RESET:
				m_pActiveCamera->reset();
				break;
			case Key::CAM_MULTIPLIER:
				m_pActiveCamera->multiplyMovementSpeed(2.0f);
				break;
			case Key::CAM_DIVIDER:
				m_pActiveCamera->divideMovementSpeed(2.0f);
				break;
			default:
				EventHandled = false;
				break;
			}
		}
		return EventHandled;
	}

	bool DemoKernel::OnKeyReleased(KeyReleasedEvent& e)
	{
		uint16_t key = e.GetKeyCode();

		if (m_debug) {
			if (m_status & DemoStatus::PAUSE)
				pauseDemo();
			else
				playDemo();
		}

		return true;
	}

	bool DemoKernel::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		return true;
	}

	bool DemoKernel::OnWindowClose(WindowCloseEvent& e)
	{
		m_exitDemo = true;
		return true;
	}

	bool DemoKernel::OnWindowResize(WindowResizeEvent& e)
	{
		if (!m_WindowResizing) {
			m_WindowResizing = true;
			m_windowWidth = static_cast<uint32_t>(std::max(e.GetWidth(), 1));
			m_windowHeight = static_cast<uint32_t>(std::max(e.GetHeight(), 1));
			m_Window->OnWindowResize(m_windowWidth, m_windowHeight);
			m_WindowResizing = false;
		}
		return true;
	}

	DemoKernel& DemoKernel::getInstance() {
		if (!kpDemoKernel)
			kpDemoKernel = new DemoKernel();
		return *kpDemoKernel;
	}

	void DemoKernel::release() {
		delete kpDemoKernel;
		kpDemoKernel = nullptr;
	}

	std::string DemoKernel::getEngineVersion()
	{
		std::stringstream ss;
		ss << PHOENIX_MAJOR_VERSION << "." << PHOENIX_MINOR_VERSION << "." << PHOENIX_BUILD_VERSION;
		return ss.str();
	}

	std::string DemoKernel::getLibAssimpVersion()
	{
		std::stringstream ss;
		ss << aiGetVersionMajor() << "." << aiGetVersionMinor() << "." << aiGetVersionRevision();
		return ss.str();
	}

	std::string DemoKernel::getLibDyadVersion()
	{
		return NetDriver::getInstance().getVersion();
	}

	// constructors/destructors

	DemoKernel::DemoKernel()
		:
		m_pText(nullptr),
		m_pActiveCamera(nullptr),
		m_pDefaultCamera(nullptr),
		m_status(-1),
		m_demoName("Phoenix Spontz Demoengine"),
		m_dataFolder("./data/"),
		m_debugFontSize(1.0f),
#ifdef _DEBUG
		m_debug(true),
		m_logLevel(LogLevel::low),
#else
		m_debug(false),
		m_logLevel(LogLevel::high),
#endif
		m_debug_drawGrid(true),
		m_debug_drawGridAxisX(true),
		m_debug_drawGridAxisY(true),
		m_debug_drawGridAxisZ(true),
		m_loop(true),
		m_sound(true),
		m_demoRunTime(0),
		m_demoStartTime(0),
		m_demoEndTime(20.0f),
		m_realFrameTime(0),
		m_frameTime(0),
		m_afterFrameTime(0),
		m_beforeFrameTime(0),
		m_accumFrameTime(0),
		m_accumFrameCount(0),
		m_fps(0),
		m_uiFrameCount(0),
		m_slaveMode(false),
		m_mouseX(0),
		m_mouseY(0),
		m_mouseXVar(0),
		m_mouseYVar(0),
		m_iLoadedSections(0),
		m_exitDemo(false),
		m_pRes(nullptr),
		m_overrideWindowConfigParams(false),
		m_windowPosX(30),
		m_windowPosY(30),
		m_windowWidth(640),
		m_windowHeight(480),
		m_WindowResizing(false)
	{
		// Create the Window management
		m_Window = std::make_unique<Window>();
		m_Window->m_demo = this; // Hack guarro

		memset(m_fVar, 0, MULTIPURPOSE_VARS * sizeof(float));
		memset(m_fBeat, 0, MAX_BEATS * sizeof(float));
	}

	DemoKernel::~DemoKernel()
	{
		delete m_pDefaultCamera;
		delete m_pRes;
	}

	// methods

	void DemoKernel::getArguments(int32_t argc, char* argv[])
	{
		std::vector <std::string> sources;
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == "-datafolder") {
				if (i + 1 < argc) { // Make sure there is another argument
					m_dataFolder = argv[++i];
				}
				else {
					std::cout << "-datafolder option requires the path to a folder." << std::endl;
				}
			}
			else if (arg == "-window") {
				if (i + 4 < argc) { // We need 4 more arguments
					m_overrideWindowConfigParams = true;
					m_windowPosX = std::stoi(argv[++i]);
					m_windowPosY = std::stoi(argv[++i]);
					m_windowWidth = std::stoi(argv[++i]);
					m_windowHeight = std::stoi(argv[++i]);
					std::cout << std::format(
						"x:{} y:{} w:{} h:{}\n",
						m_windowPosX,
						m_windowPosY,
						m_windowWidth,
						m_windowHeight
					);
				}
				else {
					std::cout << "-window option requires 4 parameters: X, Y, Width and Height. ";
					std::cout << "For example: \"-window 10 10 640 480\"." << std::endl;
				}
			}
			else {
				sources.push_back(argv[i]);
			}
		}
	}

	bool DemoKernel::initDemo()
	{
		// Window and graphics driver initializaition
		if (!m_Window->Init(m_demoName))
			return false;
		m_Window->SetEventCallback(PX_BIND_EVENT_FN(DemoKernel::OnEvent));

		Logger::info(LogLevel::med, "OpenGL environment created");

		// initialize sound driver
		if (m_sound) {
			if (BASSDRV->init())
				Logger::info(LogLevel::med, "BASS library inited");
			else
				Logger::error("Could not init BASS library");
		}

		// Show versions
		Logger::info(LogLevel::med, "Component versions:");
		Logger::info(LogLevel::high, "Spontz visuals engine 'Phoenix' version: {}", getEngineVersion());
		Logger::info(LogLevel::med, "OpenGL driver version: {}",  m_Window->getGLVersion());
		Logger::info(LogLevel::med, "OpenGL driver vendor: {}", m_Window->getGLVendor());
		Logger::info(LogLevel::med, "OpenGL driver renderer: {}", m_Window->getGLRenderer());
		Logger::info(LogLevel::med, "GLFW library version: {}", m_Window->getGLFWVersion());
		Logger::info(LogLevel::med, "Bass library version: {}", BASSDRV->getVersion());
		Logger::info(LogLevel::med, "Network Dyad.c library version: {}", getLibDyadVersion());
		Logger::info(LogLevel::med, "Assimp library version: {}", getLibAssimpVersion());

		Logger::info(LogLevel::med, "List of supported OpenGL extensions:");
		{
			Logger::ScopedIndent _;
			const std::vector<std::string> extensions = m_Window->getGLExtensions();
			for (const auto& extension : extensions)
				Logger::info(LogLevel::med, extension);
		}

		// Create the camera, by default a "projection free"
		m_pDefaultCamera = new CameraProjectionFPS(Camera::DEFAULT_CAM_POSITION);
		m_pActiveCamera = m_pDefaultCamera;

		// Start loading Basic resources
		m_pRes->loadAllResources();

		// initialize global control variables
		initControlVars();

		// Create the two embeeded layers of the engine: SectionLayer and ImGuiLayer
		m_SectionLayer = new SectionLayer(&m_sectionManager);
		PushOverlay(m_SectionLayer);

		// Load sections
		m_SectionLayer->LoadSections();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// get initial sync timer values
		initTimer();

		return true;
	}

	void DemoKernel::initNetwork()
	{
		if (m_slaveMode) {
			Logger::info(LogLevel::high, "Running in network slave mode");
			NetDriver::getInstance().init();
			NetDriver::getInstance().update();
		}
		else
			Logger::info(LogLevel::high, "Running in standalone mode");
	}

	void DemoKernel::Run()
	{
		if (m_debug)
			Logger::info(LogLevel::med, "Main demo Run loop started!");

		m_status = DemoStatus::PLAY;

		// Loop until the user closes the window
		while (!m_exitDemo) {
#ifdef PROFILE_PHOENIX
			PX_PROFILE_SCOPE("RunLoop");
#endif

			// Evaluate the time of the demo
			if (m_status == DemoStatus::PLAY) {
				// If demo is playing: Update the timing information for the sections
				processTimer();
			}
			else{
				// If the demo is not PLAYING...
				pauseTimer();
				if (m_status & DemoStatus::REWIND) {
					// decrease demo runtime
					m_demoRunTime -= 10.0f * m_realFrameTime;
					if (m_demoRunTime < m_demoStartTime) {
						m_demoRunTime = m_demoStartTime;
						pauseDemo();
					}
				}
				else if (m_status & DemoStatus::FASTFORWARD) {
					// increase demo runtime
					m_demoRunTime += 10.0f * m_realFrameTime;
					if (m_demoRunTime > m_demoEndTime) {
						m_demoRunTime = m_demoEndTime;
						pauseDemo();
					}
				}
				// reset section queues
				//reinitSectionQueues(); // TODO: Delete this
				m_SectionLayer->ReInitSections();
			}			

			// Check if demo should be ended or should be restarted
			checkDemoEnd();

			ProcessAndExecuteSections();
			ProcessAndExecuteLayers();
			ProcessAndExecuteImGUILayer();


			

			// Poll events and do SwapBuffers
			m_Window->OnUpdate();

			// update sound driver once a frame
			if (m_sound)
				BASSDRV->update();

			// Update network driver
			if (m_slaveMode)
				NetDriver::getInstance().update();

		}
	}

	void DemoKernel::doExec() // TODO: Deprecate this
	{
	/*	// control exit demo (debug, loop) when end time arrives
		if ((m_demoEndTime > 0) && (m_demoRunTime > m_demoEndTime)) {

			if (m_loop) {
				restartDemo();
			}
			else {
				if (m_debug) {
					m_demoRunTime = m_demoEndTime;
					pauseDemo();
				}
				else {
					m_exitDemo = true;
					return;
				}
			}
		}
		*/
		// non-play state
		if (m_status != DemoStatus::PLAY) {

			processSectionQueues();
			pauseTimer();
			if (m_status & DemoStatus::REWIND) {
				// decrease demo runtime
				m_demoRunTime -= 10.0f * m_realFrameTime;
				if (m_demoRunTime < m_demoStartTime) {
					m_demoRunTime = m_demoStartTime;
					pauseDemo();
				}
			}
			else if (m_status & DemoStatus::FASTFORWARD) {

				// increase demo runtime
				m_demoRunTime += 10.0f * m_realFrameTime;
				if (m_demoRunTime > m_demoEndTime) {
					m_demoRunTime = m_demoEndTime;
					pauseDemo();
				}
			}

			// reset section queues
			//reinitSectionQueues(); // TODO: Delete this
			m_SectionLayer->ReInitSections();
		}
		// play state
		else {
			// Prepare and execute the sections
			processSectionQueues();

			// Update the timing information for the sections
			processTimer();
		}

		/*
		// update sound driver once a frame
		if (m_sound)
			BASSDRV->update();

		// Update network driver
		if (m_slaveMode)
			NetDriver::getInstance().update();
		*/
	}

	void DemoKernel::playDemo()
	{
		if (m_status != DemoStatus::PLAY) {
			m_status = DemoStatus::PLAY;

			if (m_sound) BASSDRV->play();
			// reinit section queues
			//reinitSectionQueues(); // TODO: Delete this
			m_SectionLayer->ReInitSections();
		}
	}

	void DemoKernel::pauseDemo()
	{
		m_status = DemoStatus::PAUSE;
		m_frameTime = 0;
		if (m_sound) BASSDRV->pause();
	}

	void DemoKernel::restartDemo()
	{
		m_status = DemoStatus::PLAY;
		if (m_sound) {
			BASSDRV->stop();
		}

		initControlVars();
		//reinitSectionQueues(); // TODO: Delete this
		m_SectionLayer->ReInitSections();
		initTimer();
	}

	void DemoKernel::rewindDemo()
	{
		m_status = (m_status & DemoStatus::PAUSE) | DemoStatus::REWIND;
		if (m_sound) BASSDRV->stop();
	}

	void DemoKernel::fastforwardDemo()
	{
		m_status = (m_status & DemoStatus::PAUSE) | DemoStatus::FASTFORWARD;
		if (m_sound) BASSDRV->stop();
	}

	void DemoKernel::setStartTime(float theTime)
	{
		// Correct the time if it has an invalud value
		if (theTime < 0) theTime = 0;
		else if (theTime > m_demoEndTime) theTime = m_demoEndTime;

		// Set the new time
		m_demoStartTime = theTime;
	}

	void DemoKernel::setCurrentTime(float theTime)
	{
		// Correct the time if it has an invalud value
		if (theTime < 0) theTime = 0;

		// Set the new time
		m_demoRunTime = theTime;
	}

	void DemoKernel::setEndTime(float theTime)
	{
		// Correct the time if it has an invalud value
		if (theTime < 0) theTime = 0;
		else if (theTime < m_demoStartTime) theTime = m_demoStartTime;

		// Set the new time
		m_demoEndTime = theTime;
	}

	void DemoKernel::Close()
	{
		Logger::info(LogLevel::low, "Closing GL driver...");
		//GLDRV->close();				// Close GL driver

		Logger::info(LogLevel::low, "Clearing memory...");
		m_sectionManager.clear();	// Delete all sections
		m_textureManager.clear();	// Delete all textures
		m_videoManager.clear();		// Delete all videos

		m_fboManager.clearFbos();	// Clear FBO's
		m_efxBloomFbo.clearFbos();
		m_efxAccumFbo.clearFbos();

		m_modelManager.clear();		// Clear models
		m_lightManager.clear();		// Clear lights

		Logger::info(LogLevel::low, "Unloading internal resources...");
		delete m_pRes;
		m_pRes = nullptr;

		Logger::info(LogLevel::low, "Closing Bass driver...");
		BASSDRV->stop();
		BASSDRV->end();

		m_shaderManager.clear();	// Clear shaders
	}

	bool DemoKernel::checkDataFolder() const
	{
		struct stat info;
		if (stat(m_dataFolder.c_str(), &info) != 0)
			return false;
		return true;
	}

	std::string DemoKernel::getDataFolder(std::string_view path) const
	{
		return m_dataFolder + path.data();
	}

	void DemoKernel::allocateResources()
	{
		if (m_pRes == nullptr)
			m_pRes = new Resource();
	}

	bool DemoKernel::loadSpoConfig()
	{
		struct _finddata_t FindData;
		intptr_t hFile;
		std::string fullpath;
		std::string ScriptRelativePath;
		fullpath = m_dataFolder + "/config/*.spo";
		Logger::info(LogLevel::med, "Scanning config folder: {}", fullpath);
		if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
			do {
				ScriptRelativePath = m_dataFolder + "/config/" + FindData.name;
				Logger::info(LogLevel::low, "Reading file: {}", ScriptRelativePath);

				Phoenix::SpoReader spo;
				spo.readAsciiFromFile(ScriptRelativePath);
				spo.loadScriptData();
				Logger::info(LogLevel::low, "Finished loading file!");
			} while (_findnext(hFile, &FindData) == 0);
			_findclose(hFile);
		}
		else {
			Logger::error("Config files not found in 'config' folder");
			return false;
		}
		Logger::info(LogLevel::med, "Finished loading all config files.");

		// Log file
		if (m_debug)
			Logger::openLogFile();
		Logger::setLogLevel(m_logLevel);

		if (m_slaveMode) {
			Logger::info(
				LogLevel::med,
				"Engine is in slave mode, therefore, enabling force loads for shaders and textures!"
			);
			m_textureManager.forceLoad = true;
			m_shaderManager.m_forceLoad = true;
		}
		return true;
	}

	void DemoKernel::loadSpoFiles()
	{
		struct _finddata_t FindData;
		intptr_t hFile;
		std::string fullpath;
		std::string ScriptRelativePath;
		fullpath = m_dataFolder + "/*.spo";
		Logger::info(LogLevel::med, "Scanning folder: {}", fullpath);
		if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
			do {
				ScriptRelativePath = m_dataFolder + "/" + FindData.name;
				Logger::info(LogLevel::low, "Reading file: {}", ScriptRelativePath);
				SpoReader spo;
				spo.readAsciiFromFile(ScriptRelativePath);
				spo.loadScriptData();
				Logger::info(LogLevel::low, "Finished loading file!");
			} while (_findnext(hFile, &FindData) == 0);
			_findclose(hFile);
		}
		Logger::info(LogLevel::med, "Finished loading all files.");
	}

	bool DemoKernel::loadScriptFromNetwork(std::string_view sScript)
	{
		SpoReader spoReader;
		spoReader.readAsciiFromNetwork(sScript);

		const auto sectionIndex = spoReader.loadScriptData();
		if (sectionIndex < 0) {
			Logger::error("Invalid sec_id.");
			return false;
		}

		const auto pSection = m_sectionManager.m_section[static_cast<size_t>(sectionIndex)];

		// Load the data from the section
		{
			Logger::ScopedIndent _;
			pSection->loaded = pSection->load();
			if (pSection->loaded) {
				pSection->loadDebugStatic();	// Load debug static info
				Logger::info(LogLevel::low,
					"Section {} [id: {}, DataSource: {}] loaded OK!",
					sectionIndex,
					pSection->identifier,
					pSection->DataSource
				);
			}
			else {
				Logger::error(
					"Section {} [id: {}, DataSource: {}] not loaded properly!",
					sectionIndex,
					pSection->identifier,
					pSection->DataSource
				);
			}
		}

		return pSection->loaded;
	}

	void DemoKernel::initTimer()
	{
		m_beforeFrameTime = static_cast<float>(glfwGetTime());
	}

	void DemoKernel::calculateFPS(float const frameTime)
	{
		m_accumFrameTime += frameTime;
		m_accumFrameCount++;
		if (m_accumFrameTime > 0.3f) {
			m_fps = (float)m_accumFrameCount / m_accumFrameTime;
			m_accumFrameTime = 0;
			m_accumFrameCount = 0;
		}
	}

	void DemoKernel::processTimer()
	{
		// frame time calculation
		m_afterFrameTime = static_cast<float>(glfwGetTime());
		m_realFrameTime = m_afterFrameTime - m_beforeFrameTime;
		m_beforeFrameTime = m_afterFrameTime;

		// advance sections and demo time
		m_frameTime = m_realFrameTime;
		m_demoRunTime += m_frameTime;

		// frame count
		m_uiFrameCount++;

		// fps calculation
		calculateFPS(m_frameTime);
	}

	void DemoKernel::pauseTimer()
	{
		// frame time calculation
		m_afterFrameTime = static_cast<float>(glfwGetTime());
		m_realFrameTime = m_afterFrameTime - m_beforeFrameTime;
		m_beforeFrameTime = m_afterFrameTime;

		// sections should not advance
		m_frameTime = 0;

		// frame count
		m_uiFrameCount++;

		// fps calculation
		calculateFPS(m_realFrameTime);
	}

	void DemoKernel::checkDemoEnd()
	{
		// control exit demo (debug, loop) when end time arrives
		if ((m_demoEndTime > 0) && (m_demoRunTime > m_demoEndTime)) {

			if (m_loop) {
				restartDemo();
			}
			else {
				if (m_debug) {
					m_demoRunTime = m_demoEndTime;
					pauseDemo();
				}
				else {
					m_exitDemo = true;
					return;
				}
			}
		}
	}

	void DemoKernel::ProcessAndExecuteSections()
	{
		Logger::info(LogLevel::med, "Start queue processing (init and exec) for second: {:.4f}", m_demoRunTime);

		m_SectionLayer->ProcessSections(m_demoRunTime);
		{
			PX_PROFILE_SCOPE("SectionLayer Exec");

			// Prepare render
			m_Window->InitRender(true);

			// Show grid only if we are in Debug // TODO: Sacar esto a otra layer
			if (m_debug && m_debug_drawGrid) {
				PX_PROFILE_SCOPE("DrawGrid");
				m_pRes->draw3DGrid(m_debug_drawGridAxisX, m_debug_drawGridAxisY, m_debug_drawGridAxisZ);
			}

			// Set the default camera // TODO: Refactor with a function "SetDefaultCamera" or something like this
			m_pActiveCamera = m_pDefaultCamera;

			// Execute the Secions
			m_SectionLayer->ExecuteSections(m_demoRunTime);

			// Set back to the frambuffer and restore the viewport
			m_Window->SetFramebuffer();
		}
		m_SectionLayer->End();
	}

	void DemoKernel::ProcessAndExecuteLayers()
	{
		{
			PX_PROFILE_SCOPE("LayerStack OnUpdate");

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(m_realFrameTime); // Render the layers
		}
	}

	void DemoKernel::ProcessAndExecuteImGUILayer()
	{
		m_ImGuiLayer->Begin();
		{
			PX_PROFILE_SCOPE("LayerStack OnImGuiRender");

			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();			// Update the ImGui components (if any)
		}
		m_ImGuiLayer->End();
	}

	void DemoKernel::initControlVars()
	{
		// reset time
		m_demoRunTime = m_demoStartTime;

		// reset control time variables
		m_frameTime = 0;
		m_realFrameTime = 0;
		m_uiFrameCount = 0;
		m_accumFrameCount = 0;
		m_accumFrameTime = 0;
		m_fps = 0;
		m_exitDemo = false;
	}

	void DemoKernel::initSectionQueues()
	{
		Section* pSection = nullptr;
		Section* pTmpSection = nullptr;
		Section* pLoadingSection = nullptr;

		int sec_id;

		// Set the demo state to loading
		m_status = DemoStatus::LOADING;
		Logger::info(LogLevel::high, "Loading Start...");

		const auto startTime = m_debug ? static_cast<float>(glfwGetTime()) : 0.0f;

		// Search for the loading section, if not found, we will create one
		for (size_t i = 0; i < m_sectionManager.m_section.size(); i++) {
			if (m_sectionManager.m_section[i]->type == SectionType::Loading)
				pLoadingSection = m_sectionManager.m_section[i];
		}

		if (pLoadingSection == nullptr) {
			Logger::info(LogLevel::med, "Loading section not found: using default loader");
			sec_id = m_sectionManager.addSection(SectionType::Loading, "Automatically created", TRUE);
			if (sec_id < 0) {
				Logger::error("Critical Error, Loading section not found and could not be created!");
				return;
			}
			else {
				pLoadingSection = m_sectionManager.m_section[sec_id];
			}
		}

		// preload, load and init loading section
		pLoadingSection->load();
		pLoadingSection->init();
		pLoadingSection->loaded = TRUE;
		pLoadingSection->inited = TRUE;
		pLoadingSection->exec();

		{
			Logger::ScopedIndent _;
			Logger::info(LogLevel::med, "Loading section loaded, inited and executed for first time");

			// Clear the load and run section lists
			m_sectionManager.m_loadSection.clear();
			m_sectionManager.m_execSection.clear();

			// Populate Load Section: The sections that need to be loaded
			for (size_t i = 0; i < m_sectionManager.m_section.size(); i++) {
				pSection = m_sectionManager.m_section[i];
				// If we are in slave mode, we load all the sections but if not, we will load only the ones
				// that are inside the demo time
				if (m_slaveMode == 1 || ((pSection->startTime < m_demoEndTime || fabs(m_demoEndTime) < FLT_EPSILON) && (pSection->endTime > startTime))) {
					// If the section is not the "loading", then we add id to the Ready Section lst
					if (pSection->type != SectionType::Loading) {
						m_sectionManager.m_loadSection.push_back(static_cast<int32_t>(i));
						// load section splines (to avoid code load in the sections)
						// loadSplines(ds); // TODO: Delete this once splines are working
					}
				}
			}

			Logger::info(LogLevel::low,
				"Ready Section queue complete: {} sections to be loaded",
				m_sectionManager.m_loadSection.size()
			);

			// Start Loading the sections of the Ready List
			m_iLoadedSections = 0;
			for (size_t i = 0; i < m_sectionManager.m_loadSection.size(); i++) {
				sec_id = m_sectionManager.m_loadSection[i];
				pSection = m_sectionManager.m_section[sec_id];
				if (pSection->load()) {
					pSection->loadDebugStatic(); // Load static debug info
					pSection->loaded = TRUE;
				}
				// Incrmeent the loading sections even if it has not been sucesfully loaded, because
				//  it's just for the "loading" screen
				++m_iLoadedSections;

				// Update loading
				pLoadingSection->exec();
				if (pSection->loaded)
					Logger::info(LogLevel::low,
						"Section {} [id: {}, DataSource: {}] loaded OK!",
						sec_id,
						pSection->identifier,
						pSection->DataSource
					);
				else
					Logger::error(
						"Section {} [id: {}, DataSource: {}] not loaded properly!",
						sec_id,
						pSection->identifier,
						pSection->DataSource
					);

				if (m_exitDemo) {
					Close();
					exit(EXIT_SUCCESS);
				}
			}
		}

		Logger::info(LogLevel::med,
			"Loading complete, {} sections have been loaded.",
			m_iLoadedSections
		);
	}

	void DemoKernel::reinitSectionQueues()
	{
		Logger::ScopedIndent _;
		Logger::info(LogLevel::low, "Analysing sections that must be re-inited...");
		for (auto i = 0; i < m_sectionManager.m_execSection.size(); i++) {
			// The second value is the ID of the section
			const auto sec_id = m_sectionManager.m_execSection[i].second;
			const auto ds = m_sectionManager.m_section[sec_id];
			if ((ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {
				ds->inited = FALSE; // Mark the section as not inited
				Logger::info(LogLevel::low,
					"Section {} [layer: {} id: {}] marked to be inited",
					sec_id,
					ds->layer,
					ds->identifier
				);
			}
		}
	}

	void DemoKernel::processSectionQueues()
	{
		Logger::info(LogLevel::med,
			"Start queue processing (init and exec) for second: {:.4f}",
			m_demoRunTime
		);

		// Check the sections that need to be executed
		{
			Logger::ScopedIndent _;
			Logger::info(LogLevel::low, "Analysing sections that must be executed...");
			m_sectionManager.m_execSection.clear();
			for (auto i = 0; i < m_sectionManager.m_section.size(); ++i) {
				const auto pSection = m_sectionManager.m_section[i];
				// If time is OK
				if (pSection->startTime <= m_demoRunTime && pSection->endTime >= m_demoRunTime) {
					// If its enabled, loaded and is not hte Loading section
					if (pSection->enabled && pSection->loaded && pSection->type != SectionType::Loading) {
						// Load the section: first the layer and then the ID
						m_sectionManager.m_execSection.push_back(std::make_pair(pSection->layer, i));
					}
				}
			}

			// Sort sections by Layer
			sort(m_sectionManager.m_execSection.begin(), m_sectionManager.m_execSection.end());

			Logger::info(LogLevel::low,
				"Exec Section queue complete: {} sections to be executed",
				m_sectionManager.m_execSection.size()
			);
			// Run Init sections
			Logger::info(LogLevel::low, "Running Init Sections...");
			for (auto i = 0; i < m_sectionManager.m_execSection.size(); ++i) {
				// The second value is the ID of the section
				const auto sec_id = m_sectionManager.m_execSection[i].second;
				const auto ds = m_sectionManager.m_section[sec_id];
				if (ds->inited == FALSE) {
					ds->runTime = m_demoRunTime - ds->startTime;
					ds->init();			// Init the Section
					ds->inited = TRUE;
					Logger::info(LogLevel::low,
						"Section {} [layer: {} id: {} type: {}] inited",
						sec_id,
						ds->layer,
						ds->identifier,
						ds->type_str
					);
				}
			}

			// prepare engine for render
			GLDRV->initRender(true);

			// Show grid only if we are in Debug
			if (m_debug && m_debug_drawGrid) {
#ifdef PROFILE_PHOENIX
				PX_PROFILE_SCOPE("DrawGrid");
#endif
				//GLDRV->drawGrid(m_debug_drawGridAxisX, m_debug_drawGridAxisY, m_debug_drawGridAxisZ);
		}

			// Set the default camera
			m_pActiveCamera = m_pDefaultCamera;

			// Run Exec sections
			Logger::info(LogLevel::low, "Running Exec Sections...");
			{
				Logger::ScopedIndent _;
#ifdef PROFILE_PHOENIX
				PX_PROFILE_SCOPE("ExecSections");
#endif
				for (auto i = 0; i < m_sectionManager.m_execSection.size(); ++i) {
					// The second value is the ID of the section
					const auto sec_id = m_sectionManager.m_execSection[i].second;
					const auto ds = m_sectionManager.m_section[sec_id];
					ds->runTime = m_demoRunTime - ds->startTime;
					ds->exec();			// Exec the Section
					Logger::info(LogLevel::low,
						"Section {} [layer: {} id: {} type: {}] executed",
						sec_id,
						ds->layer,
						ds->identifier,
						ds->type_str
					);
				}
			}
			Logger::info(LogLevel::med, "End queue processing!");

			// Set back to the frambuffer and restore the viewport
			GLDRV->SetFramebuffer();


			// Show debug info
			if (m_debug) {
#ifdef PROFILE_PHOENIX
				PX_PROFILE_SCOPE("DrawGui");
#endif
				GLDRV->drawGui();
			}

			// swap buffer
			{
#ifdef PROFILE_PHOENIX
				PX_PROFILE_SCOPE("GLDRV::swapBuffers");
#endif
				GLDRV->swapBuffers();
			}
			}

	}
}
