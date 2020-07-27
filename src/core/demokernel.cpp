// demoKernel.cpp
// Spontz Demogroup

#include <iostream>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "main.h"
#include "core/scripting/spo.h"

// Initialize the demokernel main pointer to NULL
demokernel* demokernel::m_pThis = NULL;

// ******************************************************************

demokernel& demokernel::GetInstance() {
	static demokernel r;
	return r;
}

demokernel::demokernel() {
	// initialize global kernel variables
	this->text = nullptr;
	this->camera = nullptr;
	this->state = -1;
	this->demoName = "Phoneix Spontz Demoengine";
	this->debug = FALSE;
	this->log_detail = LogLevel::HIGH;

	#ifdef _DEBUG
	this->debug = TRUE;
	this->log_detail = LogLevel::LOW;
	#endif

	this->loop = TRUE;
	this->sound = TRUE;
	this->startTime = 0.0f;
	this->endTime = 20.0f;
	this->slaveMode = FALSE;
	this->beat = 0.0f;
	this->beat_ratio = 1.4f;
	this->beat_fadeout = 4.0f;

	// Other variables
	this->accumFrameTime = 0;
	this->accumFrameCount = 0;
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

	// Show Phoenix version
	LOG->Info(LogLevel::HIGH, "Spontz visuals engine 'Phoenix' version: %d.%d", PHOENIX_MAJOR_VERSION, PHOENIX_MINOR_VERSION);

	// initialize graphics driver
	if (!GLDRV->initGraphics())
		return false;
	LOG->Info(LogLevel::HIGH, "OpenGL environment created");
	LOG->Info(LogLevel::MED, "OpenGL library version is: %s", glGetString(GL_VERSION));

	// initialize sound driver
	if (this->sound)
		BASSDRV->init();

	// Create the camera
	this->camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

	// Start loading Basic resources
	RES->loadAllResources();

	if (DEMO->slaveMode) {
		LOG->Info(LogLevel::HIGH, "Running in network slave mode");
		NETDRV->init();
		LOG->Info(LogLevel::MED, "Network Dyad.c library version is: %s", NETDRV->getVersion());
	}
	else
		LOG->Info(LogLevel::HIGH, "Running in standalone mode");

	// Show Assimp library version
	LOG->Info(LogLevel::MED, "Assimp Library version is: %d.%d.%d", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());

	// initialize global control variables
	this->initControlVars();

	// prepare sections
	this->initSectionQueues();

	// get initial sync timer values
	this->initTimer();

	return true;
}

void demokernel::mainLoop() {
	if (this->debug)
		LOG->Info(LogLevel::MED, "************ Main demo loop started!");

	this->state = DEMO_PLAY;

	/* Loop until the user closes the window */
	while ((!GLDRV->WindowShouldClose()) && (!this->exitDemo)) {
		PX_PROFILE_SCOPE("RunLoop");

		// Poll for and process events
		GLDRV->ProcessInput();

		this->doExec();

		{
			PX_PROFILE_SCOPE("glfwPollEvents");
			glfwPollEvents();
		}
		
	}
}

void demokernel::doExec() {

	// control exit demo (debug, loop) when end time arrives
	if ((this->endTime > 0) && (this->runTime > this->endTime)) {

		if (this->loop) {
			this->restartDemo();
		}
		else {
			if (this->debug) {
				this->runTime = this->endTime;
				this->pauseDemo();
			}
			else {
				this->exitDemo = TRUE;
				return;
			}
		}
	}

	// non-play state
	if (this->state != DEMO_PLAY) {

		this->processSectionQueues();
		this->pauseTimer();
		if (this->state & DEMO_REWIND) {
			// decrease demo runtime
			this->runTime -= 10.0f * this->realFrameTime;
			if (this->runTime < this->startTime) {
				this->runTime = this->startTime;
				this->pauseDemo();
			}
		}
		else if (this->state & DEMO_FASTFORWARD) {

			// increase demo runtime
			this->runTime += 10.0f * this->realFrameTime;
			if (this->runTime > this->endTime) {
				this->runTime = this->endTime;
				this->pauseDemo();
			}
		}

		// reset section queues
		this->reInitSectionQueues();
	}
	// play state
	else {
		// Prepare and execute the sections
		this->processSectionQueues();

		// Update the timing information for the sections
		this->processTimer();
	}

	// update sound driver once a frame (seems that is not needed)
	//if (this->sound)
	//	BASSDRV->update();

	// Update network driver
	if (this->slaveMode)
		NETDRV->update();
}

void demokernel::playDemo()
{
	if (this->state != DEMO_PLAY) {
		this->state = DEMO_PLAY;

		if (this->sound) BASSDRV->play();
		// reinit section queues
		this->reInitSectionQueues();
	}
}

void demokernel::pauseDemo()
{
	this->state = DEMO_PAUSE;
	this->frameTime = 0;
	if (this->sound) BASSDRV->pause();
}

void demokernel::restartDemo()
{
	this->state = DEMO_PLAY;
	if (this->sound) {
		BASSDRV->stop();
	}

	this->initControlVars();
	this->reInitSectionQueues();
	this->initTimer();
}

void demokernel::rewindDemo()
{
	this->state = (this->state & DEMO_PAUSE) | DEMO_REWIND;
	if (this->sound) BASSDRV->stop();
}

void demokernel::fastforwardDemo()
{
	this->state = (this->state & DEMO_PAUSE) | DEMO_FASTFORWARD;
	if (this->sound) BASSDRV->stop();
}

void demokernel::setStartTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;
	else if (theTime > this->endTime) theTime = this->endTime;

	// Set the new time
	this->startTime = theTime;
}

void demokernel::setCurrentTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;

	// Set the new time
	this->runTime = theTime;
}

void demokernel::setEndTime(float theTime)
{
	// Correct the time if it has an invalud value
	if (theTime < 0) theTime = 0;
	else if (theTime < this->startTime) theTime = this->startTime;

	// Set the new time
	this->endTime = theTime;
}

void demokernel::closeDemo() {
	GLDRV->close();
}

bool demokernel::checkDataFolder()
{
	struct stat info;
	if (stat(this->dataFolder.c_str(), &info) != 0)
		return false;
	return true;
}

std::string demokernel::getFolder(std::string path)
{
	return (this->dataFolder + path);
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
	if (DEMO->debug)
		LOG->OpenLogFile();
	LOG->setLogLevel(static_cast<LogLevel>(DEMO->log_detail));

	if (DEMO->slaveMode) {
		LOG->Info(LogLevel::MED, "Engine is in slave mode, therefore, enabling force loads for shaders and textures!");
		DEMO->textureManager.forceLoad = true;
		DEMO->shaderManager.forceLoad = true;
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

	auto my_sec = this->sectionManager.section[sec_id];

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
	this->beforeFrameTime = static_cast<float>(glfwGetTime());
}

void demokernel::calculateFPS(float const frameTime)
{
	this->accumFrameTime += frameTime;
	this->accumFrameCount++;
	if (this->accumFrameTime > 0.3f) {
		this->fps = (float)this->accumFrameCount / this->accumFrameTime;
		this->accumFrameTime = 0;
		this->accumFrameCount = 0;
	}
}

void demokernel::processTimer()
{
	// frame time calculation
	this->afterFrameTime = static_cast<float>(glfwGetTime());
	this->realFrameTime = this->afterFrameTime - this->beforeFrameTime;
	this->beforeFrameTime = this->afterFrameTime;

	// advance sections and demo time
	this->frameTime = this->realFrameTime;
	this->runTime += this->frameTime;

	// frame count
	this->frameCount++;

	// fps calculation
	this->calculateFPS(this->frameTime);
}

void demokernel::pauseTimer()
{
	// frame time calculation
	this->afterFrameTime = static_cast<float>(glfwGetTime());
	this->realFrameTime = this->afterFrameTime - this->beforeFrameTime;
	this->beforeFrameTime = this->afterFrameTime;

	// sections should not advance
	this->frameTime = 0;

	// frame count
	this->frameCount++;

	// fps calculation
	this->calculateFPS(this->realFrameTime);
}

void demokernel::initControlVars() {
	// reset time
	this->runTime = this->startTime;

	// reset control time variables
	this->frameTime = 0;
	this->realFrameTime = 0;
	this->frameCount = 0;
	this->accumFrameCount = 0;
	this->accumFrameTime = 0;
	this->fps = 0;
	this->exitDemo = FALSE;
}

void demokernel::initSectionQueues() {
	Section* ds = NULL;
	Section* ds_tmp = NULL;
	Section* ds_loading = NULL;
	float startTime = 0.0f;
	int i;
	int sec_id;

	// Set the demo state to loading
	this->state = DEMO_LOADING;
	LOG->Info(LogLevel::HIGH, "Loading Start...");

	if (this->debug) {
		startTime = (float)glfwGetTime();
	}

	// Search for the loading section, if not found, we will create one
	for (i = 0; i < this->sectionManager.section.size(); i++) {
		if (this->sectionManager.section[i]->type == SectionType::Loading)
			ds_loading = this->sectionManager.section[i];
	}

	if (ds_loading == NULL) {
		LOG->Info(LogLevel::MED, "Loading section not found: using default loader");
		sec_id = this->sectionManager.addSection("Loading", "Automatically created", TRUE);
		if (sec_id < 0) {
			LOG->Error("Critical Error, Loading section not found and could not be created!");
			return;
		}
		else
			ds_loading = this->sectionManager.section[sec_id];
	}

	// preload, load and init loading section
	ds_loading->load();
	ds_loading->init();
	ds_loading->loaded = TRUE;
	ds_loading->inited = TRUE;
	ds_loading->exec();

	LOG->Info(LogLevel::MED, "  Loading section loaded, inited and executed for first time");

	// Clear the load and run section lists
	this->sectionManager.loadSection.clear();
	this->sectionManager.execSection.clear();

	// Populate Load Section: The sections that need to be loaded
	for (i = 0; i < this->sectionManager.section.size(); i++) {
		ds = this->sectionManager.section[i];
		// If we are in slave mode, we load all the sections but if not, we will load only the ones that are inside the demo time
		if ((DEMO->slaveMode == 1) || (((ds->startTime < DEMO->endTime) || fabs(DEMO->endTime) < FLT_EPSILON) && (ds->endTime > DEMO->startTime))) {
			// If the section is not the "loading", then we add id to the Ready Section lst
			if (ds->type != SectionType::Loading) {
				this->sectionManager.loadSection.push_back(i);
				// load section splines (to avoid code load in the sections)
				//loadSplines(ds); // TODO: Delete this once splines are working
			}
		}
	}

	LOG->Info(LogLevel::LOW, "  Ready Section queue complete: %d sections to be loaded", this->sectionManager.loadSection.size());

	// Start Loading the sections of the Ready List
	this->loadedSections = 0;
	for (i = 0; i < this->sectionManager.loadSection.size(); i++) {
		sec_id = this->sectionManager.loadSection[i];
		ds = this->sectionManager.section[sec_id];
		if (ds->load()) {
			ds->loaded = TRUE;
		}
		++this->loadedSections; // Incrmeent the loading sections even if it has not been sucesfully loaded, because it's just for the "loading" screen

		// Update loading
		ds_loading->exec();
		if (ds->loaded)
			LOG->Info(LogLevel::LOW, "  Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());
		else
			LOG->Error("  Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());

		if (this->exitDemo) {
			this->closeDemo();
			exit(EXIT_SUCCESS);
		}
	}

	LOG->Info(LogLevel::MED, "Loading complete, %d sections have been loaded", this->loadedSections);

	// End loading
	ds_loading->end();
	ds_loading->ended = TRUE;

}

void demokernel::reInitSectionQueues() {
	Section* ds = NULL;
	int i;
	int sec_id;

	LOG->Info(LogLevel::LOW, "  Analysing sections that must be re-inited...");
	for (i = 0; i < this->sectionManager.execSection.size(); i++) {
		sec_id = this->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = this->sectionManager.section[sec_id];
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


	LOG->Info(LogLevel::MED, "Start queue processing (end, init and exec) for second: %.4f", this->runTime);
	// We loop all the sections, searching for finished sections,
	// if any is found, we will remove from the queue and will execute the .end() function

	// Check the sections that need to be finalized
	LOG->Info(LogLevel::LOW, "  Analysing sections that can be removed...", this->runTime);
	for (it = this->sectionManager.section.begin(); it < this->sectionManager.section.end(); it++) {
		ds = *it;
		if ((ds->endTime <= this->runTime) && (ds->ended == FALSE)) {
			ds->end();
			ds->ended = TRUE;
			LOG->Info(LogLevel::LOW, "  Section [layer: %d id: %s type: %s] ended", ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}

	// Check the sections that need to be executed
	LOG->Info(LogLevel::LOW, "  Analysing sections that must be executed...", this->runTime);
	this->sectionManager.execSection.clear();
	for (i = 0; i < this->sectionManager.section.size(); i++) {
		ds = this->sectionManager.section[i];
		if ((ds->startTime <= this->runTime) && (ds->endTime >= this->runTime) &&		// If time is OK
			(ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {		// If its enabled, loaded and is not hte Loading section
			this->sectionManager.execSection.push_back(std::make_pair(ds->layer, i));		// Load the section: first the layer and then the ID
		}
	}
	sort(this->sectionManager.execSection.begin(), this->sectionManager.execSection.end());	// Sort sections by Layer

	LOG->Info(LogLevel::LOW, "  Exec Section queue complete: %d sections to be executed", this->sectionManager.execSection.size());
	// Run Init sections
	LOG->Info(LogLevel::LOW, "  Running Init Sections...");
	for (i = 0; i < this->sectionManager.execSection.size(); i++) {
		sec_id = this->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = this->sectionManager.section[sec_id];
		if (ds->inited == FALSE) {
			ds->runTime = DEMO->runTime - ds->startTime;
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
		for (i = 0; i < this->sectionManager.execSection.size(); i++) {
			sec_id = this->sectionManager.execSection[i].second;	// The second value is the ID of the section
			ds = this->sectionManager.section[sec_id];
			ds->runTime = DEMO->runTime - ds->startTime;
			ds->exec();			// Exec the Section
			LOG->Info(LogLevel::LOW, "  Section %d [layer: %d id: %s type: %s] executed", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}
	LOG->Info(LogLevel::MED, "End queue processing!");

	// Set back to the frambuffer and restore the viewport
	GLDRV->SetFramebuffer();


	// Show debug info
	if (this->debug) {
		PX_PROFILE_SCOPE("DrawGui");
		GLDRV->drawGui();
	}

	// swap buffer
	{
		PX_PROFILE_SCOPE("GLDRV::swapBuffers");
		GLDRV->swapBuffers();
	}
	
}
