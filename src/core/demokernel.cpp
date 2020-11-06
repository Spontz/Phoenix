// demoKernel.cpp
// Spontz Demogroup

#include "main.h"
#include "core/scripting/spo.h"
#include "core/drivers/gldriver.h"
#include "core/drivers/bassdriver.h"
#include "core/resource/resource.h"
#include "debug/instrumentor.h"

#include <iostream>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

// ******************************************************************

demokernel& demokernel::GetInstance() {
	static demokernel r;
	return r;
}

demokernel::demokernel()
	:
	m_pText(nullptr),
	m_pCamera(nullptr),
	m_status(-1),
	m_demoName("Phoneix Spontz Demoengine"),
	m_debug_fontSize(1.0f),
#ifdef _DEBUG
	m_debug(true),
	m_logLevel(LogLevel::low),
#else
	m_debug(false),
	m_logLevel(LogLevel::high),
#endif
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
	m_slaveMode(0),
	m_mouseX(0),
	m_mouseY(0),
	m_mouseXvar(0),
	m_mouseYvar(0),
	m_iLoadedSections(0),
	m_exitDemo(false),
	m_pRes(nullptr),
	m_videoManager(m_slaveMode==1)
{
	memset(m_fVar, 0, MULTIPURPOSE_VARS * sizeof(float));
	memset(m_fBeat, 0, MAX_BEATS * sizeof(float));
}

void demokernel::getArguments(int argc, char* argv[]) {

	if (argc > 1) {
		m_dataFolder = argv[1];
	}
	else {
		m_dataFolder = "./data/"; // Set the demo folder to the current project file (the "./" is not really required)
	}
}

bool demokernel::initDemo() {

	// initialize graphics driver
	if (!GLDRV->initGraphics())
		return false;
	Logger::info(LogLevel::high, "OpenGL environment created");

	// initialize sound driver
	if (m_sound)
		BASSDRV->init();


	// Show versions
	Logger::info(LogLevel::med, "Component versions:");
	Logger::info(LogLevel::high, "Spontz visuals engine 'Phoenix' version: %s", getEngineVersion().c_str());
	Logger::info(LogLevel::med, "OpenGL driver version is: %s", GLDRV->getOpenGLVersion().c_str());
	Logger::info(LogLevel::med, "GLFW library version is: %s", GLDRV->getVersion().c_str());
	Logger::info(LogLevel::med, "Bass library version is: %s", BASSDRV->getVersion().c_str());
	Logger::info(LogLevel::med, "Network Dyad.c library version is: %s", getLibDyadVersion().c_str());
	Logger::info(LogLevel::med, "Assimp library version is: %s", getLibAssimpVersion().c_str());

	// Create the camera
	m_pCamera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

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


void demokernel::initNetwork()
{
	if (m_slaveMode) {
		Logger::info(LogLevel::high, "Running in network slave mode");
		netDriver::GetInstance().init();
		netDriver::GetInstance().update();
	}
	else
		Logger::info(LogLevel::high, "Running in standalone mode");
}

void demokernel::mainLoop() {
	if (m_debug)
		Logger::info(LogLevel::med, "************ Main demo loop started!");

	m_status = DemoStatus::PLAY;

	/* Loop until the user closes the window */
	while ((!GLDRV->WindowShouldClose()) && (!m_exitDemo)) {
		PX_PROFILE_SCOPE("RunLoop");

		// Poll for and process events
		GLDRV->ProcessInput();

		doExec();

		{
			PX_PROFILE_SCOPE("glfwPollEvents");
			glfwPollEvents();
		}

	}
}

void demokernel::doExec() {

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
		reInitSectionQueues();
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
		netDriver::GetInstance().update();
}

void demokernel::playDemo()
{
	if (m_status != DemoStatus::PLAY) {
		m_status = DemoStatus::PLAY;

		if (m_sound) BASSDRV->play();
		// reinit section queues
		reInitSectionQueues();
	}
}

void demokernel::pauseDemo()
{
	m_status = DemoStatus::PAUSE;
	m_frameTime = 0;
	if (m_sound) BASSDRV->pause();
}

void demokernel::restartDemo()
{
	m_status = DemoStatus::PLAY;
	if (m_sound) {
		BASSDRV->stop();
	}

	initControlVars();
	reInitSectionQueues();
	initTimer();
}

void demokernel::rewindDemo()
{
	m_status = (m_status & DemoStatus::PAUSE) | DemoStatus::REWIND;
	if (m_sound) BASSDRV->stop();
}

void demokernel::fastforwardDemo()
{
	m_status = (m_status & DemoStatus::PAUSE) | DemoStatus::FASTFORWARD;
	if (m_sound) BASSDRV->stop();
}

void demokernel::setStartTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;
	else if (theTime > m_demoEndTime) theTime = m_demoEndTime;

	// Set the new time
	m_demoStartTime = theTime;
}

void demokernel::setCurrentTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;

	// Set the new time
	m_demoRunTime = theTime;
}

void demokernel::setEndTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;
	else if (theTime < m_demoStartTime) theTime = m_demoStartTime;

	// Set the new time
	m_demoEndTime = theTime;
}

void demokernel::closeDemo() {
	GLDRV->close();
}

std::string demokernel::getEngineVersion()
{
	std::stringstream ss;
	ss << PHOENIX_MAJOR_VERSION << "." << PHOENIX_MINOR_VERSION << "." << PHOENIX_BUILD_VERSION;
	return ss.str();
}

std::string demokernel::getLibAssimpVersion()
{
	std::stringstream ss;
	ss << aiGetVersionMajor() << "." << aiGetVersionMinor() << "." << aiGetVersionRevision();
	return ss.str();
}

std::string demokernel::getLibDyadVersion()
{
	return netDriver::GetInstance().getVersion();
}

bool demokernel::checkDataFolder()
{
	struct stat info;
	if (stat(m_dataFolder.c_str(), &info) != 0)
		return false;
	return true;
}

std::string demokernel::getFolder(std::string path)
{
	return (m_dataFolder + path);
}

void demokernel::allocateResources()
{
	if (m_pRes == nullptr)
		m_pRes = new Resource();
}

bool demokernel::load_config()
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
		m_shaderManager.forceLoad	= true;
	}
	return true;
}

void demokernel::load_spos()
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
			Phoenix::SpoReader spo;
			spo.readAsciiFromFile(ScriptRelativePath);
			spo.loadScriptData();
			Logger::info(LogLevel::low, "Finished loading file!");
		} while (_findnext(hFile, &FindData) == 0);
		_findclose(hFile);
	}
	Logger::info(LogLevel::med, "Finished loading all files.");
}

bool demokernel::load_scriptFromNetwork(std::string sScript)
{
	Phoenix::SpoReader spo;
	spo.readAsciiFromNetwork(sScript);

	const int sec_id = spo.loadScriptData();
	if (sec_id < 0) {
		Logger::error("Invalid sec_id.");
		return false;
	}

	auto my_sec = m_sectionManager.section[sec_id];

	// Load the data from the section
	my_sec->loaded = my_sec->load();
	if (my_sec->loaded)
		Logger::info(LogLevel::low, "  Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, my_sec->identifier.c_str(), my_sec->DataSource.c_str());
	else
		Logger::error("  Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, my_sec->identifier.c_str(), my_sec->DataSource.c_str());

	return my_sec->loaded;
}

void demokernel::initTimer()
{
	m_beforeFrameTime = static_cast<float>(glfwGetTime());
}

void demokernel::calculateFPS(float const frameTime)
{
	m_accumFrameTime += frameTime;
	m_accumFrameCount++;
	if (m_accumFrameTime > 0.3f) {
		m_fps = (float)m_accumFrameCount / m_accumFrameTime;
		m_accumFrameTime = 0;
		m_accumFrameCount = 0;
	}
}

void demokernel::processTimer()
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

void demokernel::pauseTimer()
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

void demokernel::initControlVars() {
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

void demokernel::initSectionQueues() {
	Section* ds = NULL;
	Section* ds_tmp = NULL;
	Section* ds_loading = NULL;
	float startTime = 0.0f;
	int i;
	int sec_id;

	// Set the demo state to loading
	m_status = DemoStatus::LOADING;// DEMO_LOADING;
	Logger::info(LogLevel::high, "Loading Start...");

	if (m_debug) {
		startTime = (float)glfwGetTime();
	}

	// Search for the loading section, if not found, we will create one
	for (i = 0; i < m_sectionManager.section.size(); i++) {
		if (m_sectionManager.section[i]->type == SectionType::Loading)
			ds_loading = m_sectionManager.section[i];
	}

	if (ds_loading == NULL) {
		Logger::info(LogLevel::med, "Loading section not found: using default loader");
		sec_id = m_sectionManager.addSection("Loading", "Automatically created", TRUE);
		if (sec_id < 0) {
			Logger::error("Critical Error, Loading section not found and could not be created!");
			return;
		}
		else
			ds_loading = m_sectionManager.section[sec_id];
	}

	// preload, load and init loading section
	ds_loading->load();
	ds_loading->init();
	ds_loading->loaded = TRUE;
	ds_loading->inited = TRUE;
	ds_loading->exec();

	Logger::info(LogLevel::med, "  Loading section loaded, inited and executed for first time");

	// Clear the load and run section lists
	m_sectionManager.loadSection.clear();
	m_sectionManager.execSection.clear();

	// Populate Load Section: The sections that need to be loaded
	for (i = 0; i < m_sectionManager.section.size(); i++) {
		ds = m_sectionManager.section[i];
		// If we are in slave mode, we load all the sections but if not, we will load only the ones that are inside the demo time
		if ((m_slaveMode == 1) || (((ds->startTime < m_demoEndTime) || fabs(m_demoEndTime) < FLT_EPSILON) && (ds->endTime > startTime))) {
			// If the section is not the "loading", then we add id to the Ready Section lst
			if (ds->type != SectionType::Loading) {
				m_sectionManager.loadSection.push_back(i);
				// load section splines (to avoid code load in the sections)
				//loadSplines(ds); // TODO: Delete this once splines are working
			}
		}
	}

	Logger::info(LogLevel::low, "  Ready Section queue complete: %d sections to be loaded", m_sectionManager.loadSection.size());

	// Start Loading the sections of the Ready List
	m_iLoadedSections = 0;
	for (i = 0; i < m_sectionManager.loadSection.size(); i++) {
		sec_id = m_sectionManager.loadSection[i];
		ds = m_sectionManager.section[sec_id];
		if (ds->load()) {
			ds->loaded = TRUE;
		}
		++m_iLoadedSections; // Incrmeent the loading sections even if it has not been sucesfully loaded, because it's just for the "loading" screen

		// Update loading
		ds_loading->exec();
		if (ds->loaded)
			Logger::info(LogLevel::low, "  Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());
		else
			Logger::error("  Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());

		if (m_exitDemo) {
			closeDemo();
			exit(EXIT_SUCCESS);
		}
	}

	Logger::info(LogLevel::med, "Loading complete, %d sections have been loaded", m_iLoadedSections);

	// End loading
	ds_loading->end();
	ds_loading->ended = TRUE;

}

void demokernel::reInitSectionQueues() {
	Section* ds = NULL;
	int i;
	int sec_id;

	Logger::info(LogLevel::low, "  Analysing sections that must be re-inited...");
	for (i = 0; i < m_sectionManager.execSection.size(); i++) {
		sec_id = m_sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = m_sectionManager.section[sec_id];
		if ((ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {
			ds->inited = FALSE;			// Mark the section as not inited
			Logger::info(LogLevel::low, "  Section %d [layer: %d id: %s] marked to be inited", sec_id, ds->layer, ds->identifier.c_str());
		}
	}
}

void demokernel::processSectionQueues() {
	Section* ds;
	int i;
	int sec_id;
	std::vector<Section*>::iterator it;


	Logger::info(LogLevel::med, "Start queue processing (end, init and exec) for second: %.4f", m_demoRunTime);
	// We loop all the sections, searching for finished sections,
	// if any is found, we will remove from the queue and will execute the .end() function

	// Check the sections that need to be finalized
	Logger::info(LogLevel::low, "  Analysing sections that can be removed...", m_demoRunTime);
	for (it = m_sectionManager.section.begin(); it < m_sectionManager.section.end(); it++) {
		ds = *it;
		if ((ds->endTime <= m_demoRunTime) && (ds->ended == FALSE)) {
			ds->end();
			ds->ended = TRUE;
			Logger::info(LogLevel::low, "  Section [layer: %d id: %s type: %s] ended", ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}

	// Check the sections that need to be executed
	Logger::info(LogLevel::low, "  Analysing sections that must be executed...", m_demoRunTime);
	m_sectionManager.execSection.clear();
	for (i = 0; i < m_sectionManager.section.size(); i++) {
		ds = m_sectionManager.section[i];
		if ((ds->startTime <= m_demoRunTime) && (ds->endTime >= m_demoRunTime) &&		// If time is OK
			(ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {		// If its enabled, loaded and is not hte Loading section
			m_sectionManager.execSection.push_back(std::make_pair(ds->layer, i));		// Load the section: first the layer and then the ID
		}
	}
	sort(m_sectionManager.execSection.begin(), m_sectionManager.execSection.end());	// Sort sections by Layer

	Logger::info(LogLevel::low, "  Exec Section queue complete: %d sections to be executed", m_sectionManager.execSection.size());
	// Run Init sections
	Logger::info(LogLevel::low, "  Running Init Sections...");
	for (i = 0; i < m_sectionManager.execSection.size(); i++) {
		sec_id = m_sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = m_sectionManager.section[sec_id];
		if (ds->inited == FALSE) {
			ds->runTime = m_demoRunTime - ds->startTime;
			ds->init();			// Init the Section
			ds->inited = TRUE;
			Logger::info(LogLevel::low, "  Section %d [layer: %d id: %s type: %s] inited", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}



	// prepare engine for render
	GLDRV->initRender(true);
	//GLDRV->startDrawImgGUI();

	// Run Exec sections
	Logger::info(LogLevel::low, "  Running Exec Sections...");
	{
		PX_PROFILE_SCOPE("ExecSections");
		for (i = 0; i < m_sectionManager.execSection.size(); i++) {
			sec_id = m_sectionManager.execSection[i].second;	// The second value is the ID of the section
			ds = m_sectionManager.section[sec_id];
			ds->runTime = m_demoRunTime - ds->startTime;
			ds->exec();			// Exec the Section
			Logger::info(LogLevel::low, "  Section %d [layer: %d id: %s type: %s] executed", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}
	Logger::info(LogLevel::med, "End queue processing!");

	// Set back to the frambuffer and restore the viewport
	GLDRV->SetFramebuffer();


	// Show debug info
	if (m_debug) {
		PX_PROFILE_SCOPE("DrawGui");
		GLDRV->drawGui();
	}

	// swap buffer
	{
		PX_PROFILE_SCOPE("GLDRV::swapBuffers");
		GLDRV->swapBuffers();
	}

}
