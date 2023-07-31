// Demokernel.cpp
// Spontz Demogroup

#include "main.h"
#include "core/scripting/SpoReader.h"
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
		dispatcher.Dispatch<WindowMoveEvent>(PX_BIND_EVENT_FN(DemoKernel::OnWindowMove));
		dispatcher.Dispatch<KeyPressedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnKeyReleased));
		dispatcher.Dispatch<MouseButtonPressedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnMouseButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnMouseButtonReleased));
		dispatcher.Dispatch<MouseMovedEvent>(PX_BIND_EVENT_FN(DemoKernel::OnMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(PX_BIND_EVENT_FN(DemoKernel::OnMouseScrolled));
		
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
				if (static_cast<DemoStatus>(m_status) == DemoStatus::PLAY)
					pauseDemo();
				else
					playDemo();
				break;
			case Key::RESTART:
				restartDemo();
				break;
			case Key::CAM_CAPTURE:
				if (m_cameraManager.captureActiveCameraPosition()) {
					Logger::sendEditor("Camera position saved!");
				}
				else {
					Logger::error("Camera file was not saved");
				}
				break;
			case Key::CAM_RESET:
				m_cameraManager.resetInternalCamera();
				break;
			case Key::CAM_MULTIPLIER:
				m_cameraManager.increaseInternalCameraSpeed(2.0f);
				break;
			case Key::CAM_DIVIDER:
				m_cameraManager.decreaseInternalCameraSpeed(2.0f);
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
		if (m_debug && !ImGuiWantCaptureMouse()) {
			return m_Window->OnMouseButtonPressed(e.GetMouseButton());
		}
		return true;
	}

	bool DemoKernel::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
	{
		if (m_debug && !ImGuiWantCaptureMouse()) {
			return m_Window->OnMouseButtonReleased(e.GetMouseButton());
		}
		return true;
	}

	bool DemoKernel::OnMouseMoved(MouseMovedEvent& e)
	{
		if (m_debug && !ImGuiWantCaptureMouse()) {
			return m_Window->OnMouseMoved(e.GetX(), e.GetY());
		}
		return true;
	}

	bool DemoKernel::OnMouseScrolled(MouseScrolledEvent& e)
	{
		if (m_debug && !ImGuiWantCaptureMouse()) {
			return m_Window->OnMouseScrolled(e.GetXOffset(), e.GetYOffset());
		}
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

	bool DemoKernel::OnWindowMove(WindowMoveEvent& e)
	{
		m_windowPosX = e.GetPosX();
		m_windowPosY = e.GetPosY();
		m_Window->SetWindowPos(m_windowPosX, m_windowPosY);
		return true;
	}

	void DemoKernel::OnProcessInput(float timeStep)
	{
		PX_PROFILE_FUNCTION();

		if (m_debug) {
			if (Input::IsKeyPressed(Key::CAM_FORWARD))
				m_cameraManager.processKeyInternalCamera(CameraMovement::FORWARD, timeStep);
			if (Input::IsKeyPressed(Key::CAM_BACKWARD))
				m_cameraManager.processKeyInternalCamera(CameraMovement::BACKWARD, timeStep);
			if (Input::IsKeyPressed(Key::CAM_STRAFE_LEFT))
				m_cameraManager.processKeyInternalCamera(CameraMovement::LEFT, timeStep);
			if (Input::IsKeyPressed(Key::CAM_STRAFE_RIGHT))
				m_cameraManager.processKeyInternalCamera(CameraMovement::RIGHT, timeStep);
			if (Input::IsKeyPressed(Key::CAM_ROLL_RIGHT))
				m_cameraManager.processKeyInternalCamera(CameraMovement::ROLL_RIGHT, timeStep);
			if (Input::IsKeyPressed(Key::CAM_ROLL_LEFT))
				m_cameraManager.processKeyInternalCamera(CameraMovement::ROLL_LEFT, timeStep);
			if (Input::IsKeyPressed(Key::CAM_LEFT))
				m_cameraManager.processKeyInternalCamera(CameraMovement::LEFT, timeStep);
			if (Input::IsKeyPressed(Key::CAM_RIGHT))
				m_cameraManager.processKeyInternalCamera(CameraMovement::RIGHT, timeStep);
			if (Input::IsKeyPressed(Key::CAM_UP))
				m_cameraManager.processKeyInternalCamera(CameraMovement::UP, timeStep);
			if (Input::IsKeyPressed(Key::CAM_DOWN))
				m_cameraManager.processKeyInternalCamera(CameraMovement::DOWN, timeStep);
		}

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
		m_status(-1),
		m_specialEvent(DemoSpecialEvent::NORMAL),
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
		m_debugEnableGrid(true),
		m_debugDrawGridAxisX(true),
		m_debugDrawGridAxisY(true),
		m_debugDrawGridAxisZ(true),
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
		m_exitDemo(false),
		m_pRes(nullptr),
		m_overrideWindowConfigParams(false),
		m_windowPosX(30),
		m_windowPosY(30),
		m_windowWidth(640),
		m_windowHeight(480),
		m_WindowResizing(false),
		m_ImGuiLayer(nullptr)
	{
		// Create the Window management
		m_Window = std::make_unique<Window>();
		m_Window->m_demo = this; // Hack guarro

		memset(m_fVar, 0, MULTIPURPOSE_VARS * sizeof(float));
		memset(m_fBeat, 0, MAX_BEATS * sizeof(float));
	}

	DemoKernel::~DemoKernel()
	{
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

		// Create the intenral engine camera, by default a "projection free"
		m_cameraManager.init();

		// Start loading Basic resources
		m_pRes->loadAllResources();

		// initialize global control variables
		InitControlVars();

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
			if (static_cast<DemoStatus>(m_status) == DemoStatus::PLAY) {
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
				m_SectionLayer->ReInitSections();
			}

			// If we have a SeekTime special event, force the Reinit of the sections
			if (static_cast<DemoSpecialEvent>(m_specialEvent) & DemoSpecialEvent::SEEKTIME) {
				m_SectionLayer->ReInitSections();
				m_specialEvent = m_specialEvent & ~DemoSpecialEvent::SEEKTIME;
			}
			// Check if demo should be ended or should be restarted
			checkDemoEnd();

			ProcessAndExecuteSectionsLayer();
			ProcessAndExecuteLayers();
			ProcessAndExecuteImGUILayer();


			// Poll events and do SwapBuffers
			m_Window->OnUpdate();

			// Process Input keys to control Internal Camera
			OnProcessInput(m_realFrameTime);

			// update sound driver once a frame
			if (m_sound)
				BASSDRV->update();

			// Update network driver
			if (m_slaveMode)
				NetDriver::getInstance().update();

		}
	}

	void DemoKernel::playDemo()
	{
		if (m_status != DemoStatus::PLAY) {
			m_status = DemoStatus::PLAY;

			if (m_sound) BASSDRV->play();
			// reinit section queues
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
		
		InitControlVars();
		initTimer();
		// Force an even SEEKTIME, so al sections will be re-inited
		m_specialEvent |= DemoSpecialEvent::SEEKTIME;
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
		// Correct the time if it has an invalid value
		if (theTime < 0) theTime = 0;

		// Set the new time
		m_demoRunTime = theTime;
		m_specialEvent |= DemoSpecialEvent::SEEKTIME;
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
				spo.readFromFile(ScriptRelativePath);
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
			m_textureManager.m_forceLoad = true;
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
				spo.readFromFile(ScriptRelativePath);
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
		spoReader.readFromNetwork(sScript);

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

	bool DemoKernel::ImGuiWantCaptureMouse()
	{
		if (m_ImGuiLayer == nullptr)
			return false;
		else
			return (ImGui::GetIO().WantCaptureMouse);
	}

	void DemoKernel::ImGuiAddLogMessage(std::string_view message)
	{
		if (m_ImGuiLayer)
			m_ImGuiLayer->addLog(message);
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

	void DemoKernel::ProcessAndExecuteSectionsLayer()
	{
		PX_PROFILE_FUNCTION();
		Logger::info(LogLevel::med, "Start queue processing (init and exec) for second: {:.4f}", m_demoRunTime);
		
		{
			// Process
			{
				PX_PROFILE_SCOPE("SectionsLayer::ProcessSections");
				m_SectionLayer->ProcessSections(m_demoRunTime);
			}

			// Execute
			{
				PX_PROFILE_SCOPE("SectionsLayer::ExecuteSections");

				// Prepare render
				m_Window->InitRender(true);

				// Set the internal Camera as Active, so, in case there is no active camera we can use the internal one
				m_cameraManager.setInternalCameraAsActive();

				// Execute the Secions
				m_SectionLayer->ExecuteSections(m_demoRunTime);

				// Set back to the frambuffer and restore the viewport
				m_Window->SetFramebuffer();
			}

			// End
			{
				PX_PROFILE_SCOPE("SectionsLayer::EndSections");
				m_SectionLayer->End();
			}


			// Show grid only if we are in Debug // TODO: Create a new layer for the Grid and other debug elements?
			if (m_debug && m_debugEnableGrid) {
				PX_PROFILE_SCOPE("SectionsLayer::DrawGrid (debug)");
				m_pRes->draw3DGrid(m_debugDrawGridAxisX, m_debugDrawGridAxisY, m_debugDrawGridAxisZ);
			}
		}
	}

	void DemoKernel::ProcessAndExecuteLayers()
	{
		PX_PROFILE_FUNCTION();
		for (Layer* layer : m_LayerStack)
			layer->OnUpdate(m_realFrameTime); // Render the layers
	}

	void DemoKernel::ProcessAndExecuteImGUILayer()
	{
		PX_PROFILE_FUNCTION();
		m_ImGuiLayer->Begin();
		{
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();			// Update the ImGui components (if any)
		}
		m_ImGuiLayer->End();
	}

	void DemoKernel::InitControlVars()
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

}
