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

	// globals

	DemoKernel* kpDemoKernel = nullptr;

	// static methods

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
		m_windowHeight(480)
	{
		memset(m_fVar, 0, MULTIPURPOSE_VARS * sizeof(float));
		memset(m_fBeat, 0, MAX_BEATS * sizeof(float));
	}

	DemoKernel::~DemoKernel()
	{
		delete m_pDefaultCamera;
		delete m_pRes;
	}

	// methods

	void DemoKernel::getArguments(int argc, char* argv[])
	{
		std::vector <std::string> sources;
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == "-datafolder") {
				if (i + 1 < argc) { // Make sure there is another argument
					m_dataFolder = argv[++i];
				}
				else {
					std::cout << "-datafolder option requires the path to the new folder" << std::endl;
				}
			}
			else if (arg == "-window") {
				if (i + 4 < argc) { // We need 4 more arguments
					m_overrideWindowConfigParams = true;
					m_windowPosX = std::stoi(argv[++i]);
					m_windowPosY = std::stoi(argv[++i]);
					m_windowWidth = std::stoi(argv[++i]);
					m_windowHeight = std::stoi(argv[++i]);
					std::cout << std::to_string(m_windowPosX) << std::to_string(m_windowPosY) << std::to_string(m_windowWidth) << std::to_string(m_windowHeight) << std::endl;
				}
				else {
					std::cout << "-window option requires 4 parameters: X, Y, Width and Height, for example: -window 10 10 640 480" << std::endl;
				}
			}
			else {
				sources.push_back(argv[i]);
			}
		}
	}

	bool DemoKernel::initDemo()
	{
		// initialize graphics driver
		if (!GLDRV->initGraphics())
			return false;
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
		Logger::info(LogLevel::high, "Spontz visuals engine 'Phoenix' version: %s", getEngineVersion().c_str());
		Logger::info(LogLevel::med, "OpenGL driver version: %s", GLDRV->getOpenGLVersion().c_str());
		Logger::info(LogLevel::med, "OpenGL driver vendor: %s", GLDRV->getOpenGLVendor().c_str());
		Logger::info(LogLevel::med, "OpenGL driver renderer: %s", GLDRV->getOpenGLRenderer().c_str());
		Logger::info(LogLevel::med, "GLFW library version: %s", GLDRV->getGLFWVersion().c_str());
		Logger::info(LogLevel::med, "Bass library version: %s", BASSDRV->getVersion().c_str());
		Logger::info(LogLevel::med, "Network Dyad.c library version: %s", getLibDyadVersion().c_str());
		Logger::info(LogLevel::med, "Assimp library version: %s", getLibAssimpVersion().c_str());

		Logger::info(LogLevel::med, "List of supported OpenGL extensions:");
		const std::vector<std::string> extensions = GLDRV->getOpenGLExtensions();
		for (const auto& extension : extensions)
			Logger::info(LogLevel::med, "\t%s", extension.c_str());

		// Create the camera, by default a "projection free"
		m_pDefaultCamera = new CameraProjectionFPS(Camera::DEFAULT_CAM_POSITION);
		m_pActiveCamera = m_pDefaultCamera;

		// Start loading Basic resources
		m_pRes->loadAllResources();

		// initialize global control variables
		initControlVars();

		// prepare sections
		initSectionQueues();

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

	void DemoKernel::mainLoop()
	{
		if (m_debug)
			Logger::info(LogLevel::med, "************ Main demo loop started!");

		m_status = DemoStatus::PLAY;

		/* Loop until the user closes the window */
		while ((!GLDRV->WindowShouldClose()) && (!m_exitDemo)) {
#ifdef PROFILE_PHOENIX
			PX_PROFILE_SCOPE("RunLoop");
#endif

			// Poll for and process events
			GLDRV->ProcessInput();

			doExec();

			{
#ifdef PROFILE_PHOENIX
				PX_PROFILE_SCOPE("glfwPollEvents");
#endif
				glfwPollEvents();
			}

		}
	}

	void DemoKernel::doExec()
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
			reinitSectionQueues();
		}
		// play state
		else {
			// Prepare and execute the sections
			processSectionQueues();

			// Update the timing information for the sections
			processTimer();
		}

		// update sound driver once a frame
		if (m_sound)
			BASSDRV->update();

		// Update network driver
		if (m_slaveMode)
			NetDriver::getInstance().update();
	}

	void DemoKernel::playDemo()
	{
		if (m_status != DemoStatus::PLAY) {
			m_status = DemoStatus::PLAY;

			if (m_sound) BASSDRV->play();
			// reinit section queues
			reinitSectionQueues();
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
		reinitSectionQueues();
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

	void DemoKernel::closeDemo()
	{
		Logger::info(LogLevel::low, "Closing GL driver...");
		GLDRV->close();				// Close GL driver

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
		Logger::info(LogLevel::med, "Scanning config folder: %s", fullpath.c_str());
		if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
			do {
				ScriptRelativePath = m_dataFolder + "/config/" + FindData.name;
				Logger::info(LogLevel::low, "Reading file: %s", ScriptRelativePath.c_str());

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
			Logger::info(LogLevel::med, "Engine is in slave mode, therefore, enabling force loads for shaders and textures!");
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
		Logger::info(LogLevel::med, "Scanning folder: %s", fullpath.c_str());
		if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
			do {
				ScriptRelativePath = m_dataFolder + "/" + FindData.name;
				Logger::info(LogLevel::low, "Reading file: %s", ScriptRelativePath.c_str());
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
				Logger::info(
					LogLevel::low,
					"Section %d [id: %s, DataSource: %s] loaded OK!",
					sectionIndex,
					pSection->identifier.c_str(),
					pSection->DataSource.c_str()
				);
			}
			else {
				Logger::error(
					"Section %d [id: %s, DataSource: %s] not loaded properly!",
					sectionIndex,
					pSection->identifier.c_str(),
					pSection->DataSource.c_str()
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
				// If we are in slave mode, we load all the sections but if not, we will load only the ones that are inside the demo time
				if ((m_slaveMode == 1) || (((pSection->startTime < m_demoEndTime) || fabs(m_demoEndTime) < FLT_EPSILON) && (pSection->endTime > startTime))) {
					// If the section is not the "loading", then we add id to the Ready Section lst
					if (pSection->type != SectionType::Loading) {
						m_sectionManager.m_loadSection.push_back(static_cast<int32_t>(i));
						// load section splines (to avoid code load in the sections)
						//loadSplines(ds); // TODO: Delete this once splines are working
					}
				}
			}

			Logger::info(LogLevel::low, "Ready Section queue complete: %zu sections to be loaded", m_sectionManager.m_loadSection.size());

			// Start Loading the sections of the Ready List
			m_iLoadedSections = 0;
			for (size_t i = 0; i < m_sectionManager.m_loadSection.size(); i++) {
				sec_id = m_sectionManager.m_loadSection[i];
				pSection = m_sectionManager.m_section[sec_id];
				if (pSection->load()) {
					pSection->loadDebugStatic(); // Load static debug info
					pSection->loaded = TRUE;
				}
				++m_iLoadedSections; // Incrmeent the loading sections even if it has not been sucesfully loaded, because it's just for the "loading" screen

				// Update loading
				pLoadingSection->exec();
				if (pSection->loaded)
					Logger::info(LogLevel::low, "Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, pSection->identifier.c_str(), pSection->DataSource.c_str());
				else
					Logger::error("Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, pSection->identifier.c_str(), pSection->DataSource.c_str());

				if (m_exitDemo) {
					closeDemo();
					exit(EXIT_SUCCESS);
				}
			}
		}

		Logger::info(LogLevel::med, "Loading complete, %d sections have been loaded", m_iLoadedSections);
	}

	void DemoKernel::reinitSectionQueues()
	{
		Logger::ScopedIndent _;
		Logger::info(LogLevel::low, "Analysing sections that must be re-inited...");
		for (auto i = 0; i < m_sectionManager.m_execSection.size(); i++) {
			const auto sec_id = m_sectionManager.m_execSection[i].second;	// The second value is the ID of the section
			const auto ds = m_sectionManager.m_section[sec_id];
			if ((ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {
				ds->inited = FALSE;			// Mark the section as not inited
				Logger::info(LogLevel::low, "Section %d [layer: %d id: %s] marked to be inited", sec_id, ds->layer, ds->identifier.c_str());
			}
		}
	}

	void DemoKernel::processSectionQueues()
	{
		Logger::info(LogLevel::med, "Start queue processing (init and exec) for second: %.4f", m_demoRunTime);

		// Check the sections that need to be executed
		{
			Logger::ScopedIndent _;
			Logger::info(LogLevel::low, "Analysing sections that must be executed...", m_demoRunTime);
			m_sectionManager.m_execSection.clear();
			for (auto i = 0; i < m_sectionManager.m_section.size(); ++i) {
				const auto pSection = m_sectionManager.m_section[i];
				if ((pSection->startTime <= m_demoRunTime) && (pSection->endTime >= m_demoRunTime) &&		// If time is OK
					(pSection->enabled) && (pSection->loaded) && (pSection->type != SectionType::Loading)) {		// If its enabled, loaded and is not hte Loading section
					m_sectionManager.m_execSection.push_back(std::make_pair(pSection->layer, i));		// Load the section: first the layer and then the ID
				}
			}
			sort(m_sectionManager.m_execSection.begin(), m_sectionManager.m_execSection.end());	// Sort sections by Layer

			Logger::info(LogLevel::low, "Exec Section queue complete: %zu sections to be executed", m_sectionManager.m_execSection.size());
			// Run Init sections
			Logger::info(LogLevel::low, "Running Init Sections...");
			for (auto i = 0; i < m_sectionManager.m_execSection.size(); ++i) {
				const auto sec_id = m_sectionManager.m_execSection[i].second;	// The second value is the ID of the section
				const auto ds = m_sectionManager.m_section[sec_id];
				if (ds->inited == FALSE) {
					ds->runTime = m_demoRunTime - ds->startTime;
					ds->init();			// Init the Section
					ds->inited = TRUE;
					Logger::info(LogLevel::low, "Section %d [layer: %d id: %s type: %s] inited", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
				}
			}

			// prepare engine for render
			GLDRV->initRender(1);

			// Show grid only if we are in Debug
			if (m_debug && m_debug_drawGrid) {
#ifdef PROFILE_PHOENIX
				PX_PROFILE_SCOPE("DrawGrid");
#endif
				GLDRV->drawGrid(m_debug_drawGridAxisX, m_debug_drawGridAxisY, m_debug_drawGridAxisZ);
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
					const auto sec_id = m_sectionManager.m_execSection[i].second;	// The second value is the ID of the section
					const auto ds = m_sectionManager.m_section[sec_id];
					ds->runTime = m_demoRunTime - ds->startTime;
					ds->exec();			// Exec the Section
					Logger::info(
						LogLevel::low,
						"Section %d [layer: %d id: %s type: %s] executed",
						sec_id,
						ds->layer,
						ds->identifier.c_str(),
						ds->type_str.c_str()
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