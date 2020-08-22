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
	text (nullptr),
	camera (nullptr),
	state(-1),
	demoName("Phoneix Spontz Demoengine"),
	debug (false),
	debug_fontSize(1.0f),
	log_detail (LogLevel::HIGH),
	loop (true),
	sound (true),
	demo_runTime (0),
	demo_startTime (0),
	demo_endTime (20.0f),
	realFrameTime(0),
	frameTime(0),
	afterFrameTime(0),
	beforeFrameTime(0),
	accumFrameTime(0),
	accumFrameCount(0),
	fps(0),
	frameCount(0),
	slaveMode (false),
	beat (0),
	beat_ratio (1.4f),
	beat_fadeout (4.0f),
	mouseX(0),
	mouseY(0),
	mouseXvar(0),
	mouseYvar(0),
	loadedSections(0),
	exitDemo(false),
	res (nullptr)
{
	#ifdef _DEBUG
	debug = TRUE;
	log_detail = LogLevel::LOW;
	#endif
	for (int i = 0; i < MULTIPURPOSE_VARS; i++)
		var[i] = 0;
}

void demokernel::getArguments(int argc, char* argv[]) {

	if (argc > 1) {
		dataFolder = argv[1];
	}
	else {
		dataFolder = "./data/"; // Set the demo folder to the current project file (the "./" is not really required)
	}
}

bool demokernel::initDemo() {

	// initialize graphics driver
	if (!GLDRV->initGraphics())
		return false;
	LOG->Info(LogLevel::HIGH, "OpenGL environment created");
	
	// initialize sound driver
	if (sound)
		BASSDRV->init();


	// Show versions
	LOG->Info(LogLevel::MED, "Component versions:");
	LOG->Info(LogLevel::HIGH, "Spontz visuals engine 'Phoenix' version: %s", getEngineVersion().c_str());
	LOG->Info(LogLevel::MED, "OpenGL driver version is: %s", GLDRV->getOpenGLVersion().c_str());
	LOG->Info(LogLevel::MED, "GLFW library version is: %s", GLDRV->getVersion().c_str());
	LOG->Info(LogLevel::MED, "Bass library version is: %s", BASSDRV->getVersion().c_str());
	LOG->Info(LogLevel::MED, "Network Dyad.c library version is: %s", getLibDyadVersion().c_str());
	LOG->Info(LogLevel::MED, "Assimp library version is: %s", getLibAssimpVersion().c_str());

	// Create the camera
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

	// Start loading Basic resources
	res->loadAllResources();

	if (slaveMode) {
		LOG->Info(LogLevel::HIGH, "Running in network slave mode");
		NETDRV->init();
		
	}
	else
		LOG->Info(LogLevel::HIGH, "Running in standalone mode");
	// initialize global control variables
	initControlVars();

	// prepare sections
	initSectionQueues();

	// get initial sync timer values
	initTimer();

	return true;
}

void demokernel::mainLoop() {
	if (debug)
		LOG->Info(LogLevel::MED, "************ Main demo loop started!");

	state = DemoStatus::PLAY;

	/* Loop until the user closes the window */
	while ((!GLDRV->WindowShouldClose()) && (!exitDemo)) {
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
	if ((demo_endTime > 0) && (demo_runTime > demo_endTime)) {

		if (loop) {
			restartDemo();
		}
		else {
			if (debug) {
				demo_runTime = demo_endTime;
				pauseDemo();
			}
			else {
				exitDemo = TRUE;
				return;
			}
		}
	}

	// non-play state
	if (state != DemoStatus::PLAY) {

		processSectionQueues();
		pauseTimer();
		if (state & DemoStatus::REWIND) {
			// decrease demo runtime
			demo_runTime -= 10.0f * realFrameTime;
			if (demo_runTime < demo_startTime) {
				demo_runTime = demo_startTime;
				pauseDemo();
			}
		}
		else if (state & DemoStatus::FASTFORWARD) {

			// increase demo runtime
			demo_runTime += 10.0f * realFrameTime;
			if (demo_runTime > demo_endTime) {
				demo_runTime = demo_endTime;
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

	// update sound driver once a frame (seems that is not needed)
	//if (sound)
	//	BASSDRV->update();

	// Update network driver
	if (slaveMode)
		NETDRV->update();
}

void demokernel::playDemo()
{
	if (state != DemoStatus::PLAY) {
		state = DemoStatus::PLAY;

		if (sound) BASSDRV->play();
		// reinit section queues
		reInitSectionQueues();
	}
}

void demokernel::pauseDemo()
{
	state = DemoStatus::PAUSE;
	frameTime = 0;
	if (sound) BASSDRV->pause();
}

void demokernel::restartDemo()
{
	state = DemoStatus::PLAY;
	if (sound) {
		BASSDRV->stop();
	}

	initControlVars();
	reInitSectionQueues();
	initTimer();
}

void demokernel::rewindDemo()
{
	state = (state & DemoStatus::PAUSE) | DemoStatus::REWIND;
	if (sound) BASSDRV->stop();
}

void demokernel::fastforwardDemo()
{
	state = (state & DemoStatus::PAUSE) | DemoStatus::FASTFORWARD;
	if (sound) BASSDRV->stop();
}

void demokernel::setStartTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;
	else if (theTime > demo_endTime) theTime = demo_endTime;

	// Set the new time
	demo_startTime = theTime;
}

void demokernel::setCurrentTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;

	// Set the new time
	demo_runTime = theTime;
}

void demokernel::setEndTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;
	else if (theTime < demo_startTime) theTime = demo_startTime;

	// Set the new time
	demo_endTime = theTime;
}

void demokernel::closeDemo() {
	GLDRV->close();
}

const std::string demokernel::getEngineVersion()
{
	return std::string(std::to_string(PHOENIX_MAJOR_VERSION) + "." + std::to_string(PHOENIX_MINOR_VERSION) + "." + std::to_string(PHOENIX_BUILD_VERSION));
}

const std::string demokernel::getLibAssimpVersion()
{
	return std::string(std::to_string(aiGetVersionMajor()) +"."+ std::to_string(aiGetVersionMinor()) + "." + std::to_string(aiGetVersionRevision()));
}

const std::string demokernel::getLibDyadVersion()
{
	return NETDRV->getVersion();
}

bool demokernel::checkDataFolder()
{
	struct stat info;
	if (stat(dataFolder.c_str(), &info) != 0)
		return false;
	return true;
}

std::string demokernel::getFolder(std::string path)
{
	return (dataFolder + path);
}

void demokernel::allocateResources()
{
	if (res == nullptr)
		res = new Resource();
}

bool demokernel::load_config()
{
	struct _finddata_t FindData;
	intptr_t hFile;
	std::string fullpath;
	std::string ScriptRelativePath;
	fullpath = dataFolder + "/config/*.spo";
	LOG->Info(LogLevel::MED, "Scanning config folder: %s", fullpath.c_str());
	if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
		do {
			ScriptRelativePath = dataFolder + "/config/" + FindData.name;
			LOG->Info(LogLevel::LOW, "Reading file: %s", ScriptRelativePath.c_str());
			
			Phoenix::SpoReader spo;
			spo.readAsciiFromFile(ScriptRelativePath);
			spo.loadScriptData();
			LOG->Info(LogLevel::LOW, "Finished loading file!");
		} while (_findnext(hFile, &FindData) == 0);
		_findclose(hFile);
	}
	else {
		LOG->Error("Config files not found in 'config' folder");
		return false;
	}
	LOG->Info(LogLevel::MED, "Finished loading all config files.");

	// Log file
	if (debug)
		LOG->OpenLogFile();
	LOG->setLogLevel(static_cast<LogLevel>(log_detail));

	if (slaveMode) {
		LOG->Info(LogLevel::MED, "Engine is in slave mode, therefore, enabling force loads for shaders and textures!");
		textureManager.forceLoad = true;
		shaderManager.forceLoad = true;
	}
	return true;
}

void demokernel::load_spos()
{
	struct _finddata_t FindData;
	intptr_t hFile;
	std::string fullpath;
	std::string ScriptRelativePath;
	fullpath = dataFolder + "/*.spo";
	LOG->Info(LogLevel::MED, "Scanning folder: %s", fullpath.c_str());
	if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
		do {
			ScriptRelativePath = dataFolder + "/" + FindData.name;
			LOG->Info(LogLevel::LOW, "Reading file: %s", ScriptRelativePath.c_str());
			Phoenix::SpoReader spo;
			spo.readAsciiFromFile(ScriptRelativePath);
			spo.loadScriptData();			
			LOG->Info(LogLevel::LOW, "Finished loading file!");
		} while (_findnext(hFile, &FindData) == 0);
		_findclose(hFile);
	}
	LOG->Info(LogLevel::MED, "Finished loading all files.");
}

bool demokernel::load_scriptFromNetwork(std::string sScript)
{
	Phoenix::SpoReader spo;
	spo.readAsciiFromNetwork(sScript);

	const int sec_id = spo.loadScriptData();
	if (sec_id < 0) {
		LOG->Error("Invalid sec_id.");
		return false;
	}

	auto my_sec = sectionManager.section[sec_id];

	// Load the data from the section
	my_sec->loaded = my_sec->load();
	if (my_sec->loaded)
		LOG->Info(LogLevel::LOW, "  Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, my_sec->identifier.c_str(), my_sec->DataSource.c_str());
	else
		LOG->Error("  Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, my_sec->identifier.c_str(), my_sec->DataSource.c_str());

	return my_sec->loaded;
}

void demokernel::initTimer()
{
	beforeFrameTime = static_cast<float>(glfwGetTime());
}

void demokernel::calculateFPS(float const frameTime)
{
	accumFrameTime += frameTime;
	accumFrameCount++;
	if (accumFrameTime > 0.3f) {
		fps = (float)accumFrameCount / accumFrameTime;
		accumFrameTime = 0;
		accumFrameCount = 0;
	}
}

void demokernel::processTimer()
{
	// frame time calculation
	afterFrameTime = static_cast<float>(glfwGetTime());
	realFrameTime = afterFrameTime - beforeFrameTime;
	beforeFrameTime = afterFrameTime;

	// advance sections and demo time
	frameTime = realFrameTime;
	demo_runTime += frameTime;

	// frame count
	frameCount++;

	// fps calculation
	calculateFPS(frameTime);
}

void demokernel::pauseTimer()
{
	// frame time calculation
	afterFrameTime = static_cast<float>(glfwGetTime());
	realFrameTime = afterFrameTime - beforeFrameTime;
	beforeFrameTime = afterFrameTime;

	// sections should not advance
	frameTime = 0;

	// frame count
	frameCount++;

	// fps calculation
	calculateFPS(realFrameTime);
}

void demokernel::initControlVars() {
	// reset time
	demo_runTime = demo_startTime;

	// reset control time variables
	frameTime = 0;
	realFrameTime = 0;
	frameCount = 0;
	accumFrameCount = 0;
	accumFrameTime = 0;
	fps = 0;
	exitDemo = FALSE;
}

void demokernel::initSectionQueues() {
	Section* ds = NULL;
	Section* ds_tmp = NULL;
	Section* ds_loading = NULL;
	float startTime = 0.0f;
	int i;
	int sec_id;

	// Set the demo state to loading
	state = DemoStatus::LOADING;// DEMO_LOADING;
	LOG->Info(LogLevel::HIGH, "Loading Start...");

	if (debug) {
		startTime = (float)glfwGetTime();
	}

	// Search for the loading section, if not found, we will create one
	for (i = 0; i < sectionManager.section.size(); i++) {
		if (sectionManager.section[i]->type == SectionType::Loading)
			ds_loading = sectionManager.section[i];
	}

	if (ds_loading == NULL) {
		LOG->Info(LogLevel::MED, "Loading section not found: using default loader");
		sec_id = sectionManager.addSection("Loading", "Automatically created", TRUE);
		if (sec_id < 0) {
			LOG->Error("Critical Error, Loading section not found and could not be created!");
			return;
		}
		else
			ds_loading = sectionManager.section[sec_id];
	}

	// preload, load and init loading section
	ds_loading->load();
	ds_loading->init();
	ds_loading->loaded = TRUE;
	ds_loading->inited = TRUE;
	ds_loading->exec();

	LOG->Info(LogLevel::MED, "  Loading section loaded, inited and executed for first time");

	// Clear the load and run section lists
	sectionManager.loadSection.clear();
	sectionManager.execSection.clear();

	// Populate Load Section: The sections that need to be loaded
	for (i = 0; i < sectionManager.section.size(); i++) {
		ds = sectionManager.section[i];
		// If we are in slave mode, we load all the sections but if not, we will load only the ones that are inside the demo time
		if ((slaveMode == 1) || (((ds->startTime < demo_endTime) || fabs(demo_endTime) < FLT_EPSILON) && (ds->endTime > startTime))) {
			// If the section is not the "loading", then we add id to the Ready Section lst
			if (ds->type != SectionType::Loading) {
				sectionManager.loadSection.push_back(i);
				// load section splines (to avoid code load in the sections)
				//loadSplines(ds); // TODO: Delete this once splines are working
			}
		}
	}

	LOG->Info(LogLevel::LOW, "  Ready Section queue complete: %d sections to be loaded", sectionManager.loadSection.size());

	// Start Loading the sections of the Ready List
	loadedSections = 0;
	for (i = 0; i < sectionManager.loadSection.size(); i++) {
		sec_id = sectionManager.loadSection[i];
		ds = sectionManager.section[sec_id];
		if (ds->load()) {
			ds->loaded = TRUE;
		}
		++loadedSections; // Incrmeent the loading sections even if it has not been sucesfully loaded, because it's just for the "loading" screen

		// Update loading
		ds_loading->exec();
		if (ds->loaded)
			LOG->Info(LogLevel::LOW, "  Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());
		else
			LOG->Error("  Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());

		if (exitDemo) {
			closeDemo();
			exit(EXIT_SUCCESS);
		}
	}

	LOG->Info(LogLevel::MED, "Loading complete, %d sections have been loaded", loadedSections);

	// End loading
	ds_loading->end();
	ds_loading->ended = TRUE;

}

void demokernel::reInitSectionQueues() {
	Section* ds = NULL;
	int i;
	int sec_id;

	LOG->Info(LogLevel::LOW, "  Analysing sections that must be re-inited...");
	for (i = 0; i < sectionManager.execSection.size(); i++) {
		sec_id = sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = sectionManager.section[sec_id];
		if ((ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {
			ds->inited = FALSE;			// Mark the section as not inited
			LOG->Info(LogLevel::LOW, "  Section %d [layer: %d id: %s] marked to be inited", sec_id, ds->layer, ds->identifier.c_str());
		}
	}
}

void demokernel::processSectionQueues() {
	Section* ds;
	int i;
	int sec_id;
	std::vector<Section*>::iterator it;


	LOG->Info(LogLevel::MED, "Start queue processing (end, init and exec) for second: %.4f", demo_runTime);
	// We loop all the sections, searching for finished sections,
	// if any is found, we will remove from the queue and will execute the .end() function

	// Check the sections that need to be finalized
	LOG->Info(LogLevel::LOW, "  Analysing sections that can be removed...", demo_runTime);
	for (it = sectionManager.section.begin(); it < sectionManager.section.end(); it++) {
		ds = *it;
		if ((ds->endTime <= demo_runTime) && (ds->ended == FALSE)) {
			ds->end();
			ds->ended = TRUE;
			LOG->Info(LogLevel::LOW, "  Section [layer: %d id: %s type: %s] ended", ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}

	// Check the sections that need to be executed
	LOG->Info(LogLevel::LOW, "  Analysing sections that must be executed...", demo_runTime);
	sectionManager.execSection.clear();
	for (i = 0; i < sectionManager.section.size(); i++) {
		ds = sectionManager.section[i];
		if ((ds->startTime <= demo_runTime) && (ds->endTime >= demo_runTime) &&		// If time is OK
			(ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {		// If its enabled, loaded and is not hte Loading section
			sectionManager.execSection.push_back(std::make_pair(ds->layer, i));		// Load the section: first the layer and then the ID
		}
	}
	sort(sectionManager.execSection.begin(), sectionManager.execSection.end());	// Sort sections by Layer

	LOG->Info(LogLevel::LOW, "  Exec Section queue complete: %d sections to be executed", sectionManager.execSection.size());
	// Run Init sections
	LOG->Info(LogLevel::LOW, "  Running Init Sections...");
	for (i = 0; i < sectionManager.execSection.size(); i++) {
		sec_id = sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = sectionManager.section[sec_id];
		if (ds->inited == FALSE) {
			ds->runTime = demo_runTime - ds->startTime;
			ds->init();			// Init the Section
			ds->inited = TRUE;
			LOG->Info(LogLevel::LOW, "  Section %d [layer: %d id: %s type: %s] inited", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}



	// prepare engine for render
	GLDRV->initRender(true);
	//GLDRV->startDrawImgGUI();

	// Run Exec sections
	LOG->Info(LogLevel::LOW, "  Running Exec Sections...");
	{
		PX_PROFILE_SCOPE("ExecSections");
		for (i = 0; i < sectionManager.execSection.size(); i++) {
			sec_id = sectionManager.execSection[i].second;	// The second value is the ID of the section
			ds = sectionManager.section[sec_id];
			ds->runTime = demo_runTime - ds->startTime;
			ds->exec();			// Exec the Section
			LOG->Info(LogLevel::LOW, "  Section %d [layer: %d id: %s type: %s] executed", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}
	LOG->Info(LogLevel::MED, "End queue processing!");

	// Set back to the frambuffer and restore the viewport
	GLDRV->SetFramebuffer();


	// Show debug info
	if (debug) {
		PX_PROFILE_SCOPE("DrawGui");
		GLDRV->drawGui();
	}

	// swap buffer
	{
		PX_PROFILE_SCOPE("GLDRV::swapBuffers");
		GLDRV->swapBuffers();
	}
	
}
