// demoKernel.cpp
// Spontz Demogroup

#include <iostream>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "main.h"


using namespace std;

// Initialize the demokernel main pointer to NULL
demokernel* demokernel::m_pThis = NULL;

// ******************************************************************
// scripting
// ******************************************************************

#define VTYPE_INT		0
#define VTYPE_FLOAT		1
#define VTYPE_STRING	2

typedef struct {
	char *cName;
	char vType;
	void *vAddr;
} tScriptCommand;

static tScriptCommand scriptCommand[] = {

	{"demo_name",		VTYPE_STRING,		&DEMO->demoName			},
	{"debug",			VTYPE_INT,			&DEMO->debug			},
	{"record",			VTYPE_INT,			&DEMO->record			},
	{"record_fps",		VTYPE_FLOAT,		&DEMO->recordFps		},
	{"compress_tga",	VTYPE_INT,			&DEMO->compressTga		},
	{"loop",			VTYPE_INT,			&DEMO->loop				},
	{"sound",			VTYPE_INT,			&DEMO->sound			},
	{"bench",			VTYPE_INT,			&DEMO->bench			},
	{"demo_start",		VTYPE_FLOAT,		&DEMO->startTime		},
	{"demo_end",		VTYPE_FLOAT,		&DEMO->endTime			},
	{"slave",			VTYPE_INT,			&DEMO->slaveMode		},

	{"gl_fullscreen",	VTYPE_INT,			&GLDRV->fullScreen		},
	{"gl_info",			VTYPE_INT,			&GLDRV->saveInfo		},
	{"gl_width",		VTYPE_INT,			&GLDRV->width			},
	{"gl_height",		VTYPE_INT,			&GLDRV->height			},
	{"gl_bpp",			VTYPE_INT,			&GLDRV->bpp				},
	{"gl_stencil",		VTYPE_INT,			&GLDRV->stencil			},
	{"gl_accum",		VTYPE_INT,			&GLDRV->accum			},
	{"gl_multisampling",VTYPE_INT,			&GLDRV->multisampling	},

	{"gl_gamma",		VTYPE_FLOAT,		&GLDRV->gamma			},

	{"fbo_0_ratio",		VTYPE_INT,			&GLDRV->fbo[0].ratio	},
	{"fbo_0_format",	VTYPE_STRING,		&GLDRV->fbo[0].format	},
	{"fbo_1_ratio",		VTYPE_INT,			&GLDRV->fbo[1].ratio	},
	{"fbo_1_format",	VTYPE_STRING,		&GLDRV->fbo[1].format	},
	{"fbo_2_ratio",		VTYPE_INT,			&GLDRV->fbo[2].ratio	},
	{"fbo_2_format",	VTYPE_STRING,		&GLDRV->fbo[2].format	},
	{"fbo_3_ratio",		VTYPE_INT,			&GLDRV->fbo[3].ratio	},
	{"fbo_3_format",	VTYPE_STRING,		&GLDRV->fbo[3].format	},
	{"fbo_4_ratio",		VTYPE_INT,			&GLDRV->fbo[4].ratio	},
	{"fbo_4_format",	VTYPE_STRING,		&GLDRV->fbo[4].format	},
	{"fbo_5_ratio",		VTYPE_INT,			&GLDRV->fbo[5].ratio	},
	{"fbo_5_format",	VTYPE_STRING,		&GLDRV->fbo[5].format	},
	{"fbo_6_ratio",		VTYPE_INT,			&GLDRV->fbo[6].ratio	},
	{"fbo_6_format",	VTYPE_STRING,		&GLDRV->fbo[6].format	},
	{"fbo_7_ratio",		VTYPE_INT,			&GLDRV->fbo[7].ratio	},
	{"fbo_7_format",	VTYPE_STRING,		&GLDRV->fbo[7].format	},
	{"fbo_8_ratio",		VTYPE_INT,			&GLDRV->fbo[8].ratio	},
	{"fbo_8_format",	VTYPE_STRING,		&GLDRV->fbo[8].format	},
	{"fbo_9_ratio",		VTYPE_INT,			&GLDRV->fbo[9].ratio	},
	{"fbo_9_format",	VTYPE_STRING,		&GLDRV->fbo[9].format	},
	{"fbo_10_ratio",	VTYPE_INT,			&GLDRV->fbo[10].ratio	},
	{"fbo_10_format",	VTYPE_STRING,		&GLDRV->fbo[10].format	},
	{"fbo_11_ratio",	VTYPE_INT,			&GLDRV->fbo[11].ratio	},
	{"fbo_11_format",	VTYPE_STRING,		&GLDRV->fbo[11].format	},
	{"fbo_12_ratio",	VTYPE_INT,			&GLDRV->fbo[12].ratio	},
	{"fbo_12_format",	VTYPE_STRING,		&GLDRV->fbo[12].format	},
	{"fbo_13_ratio",	VTYPE_INT,			&GLDRV->fbo[13].ratio	},
	{"fbo_13_format",	VTYPE_STRING,		&GLDRV->fbo[13].format	},
	{"fbo_14_ratio",	VTYPE_INT,			&GLDRV->fbo[14].ratio	},
	{"fbo_14_format",	VTYPE_STRING,		&GLDRV->fbo[14].format	},
	{"fbo_15_ratio",	VTYPE_INT,			&GLDRV->fbo[15].ratio	},
	{"fbo_15_format",	VTYPE_STRING,		&GLDRV->fbo[15].format	},
	{"fbo_16_ratio",	VTYPE_INT,			&GLDRV->fbo[16].ratio	},
	{"fbo_16_format",	VTYPE_STRING,		&GLDRV->fbo[16].format	},
	{"fbo_17_ratio",	VTYPE_INT,			&GLDRV->fbo[17].ratio	},
	{"fbo_17_format",	VTYPE_STRING,		&GLDRV->fbo[17].format	},
	{"fbo_18_ratio",	VTYPE_INT,			&GLDRV->fbo[18].ratio	},
	{"fbo_18_format",	VTYPE_STRING,		&GLDRV->fbo[18].format	},
	{"fbo_19_ratio",	VTYPE_INT,			&GLDRV->fbo[19].ratio	},
	{"fbo_19_format",	VTYPE_STRING,		&GLDRV->fbo[19].format	},

	{"fbo_20_width",	VTYPE_INT,			&GLDRV->fbo[20].width	},
	{"fbo_20_height",	VTYPE_INT,			&GLDRV->fbo[20].height	},
	{"fbo_20_format",	VTYPE_STRING,		&GLDRV->fbo[20].format	},
	{"fbo_21_width",	VTYPE_INT,			&GLDRV->fbo[21].width	},
	{"fbo_21_height",	VTYPE_INT,			&GLDRV->fbo[21].height	},
	{"fbo_21_format",	VTYPE_STRING,		&GLDRV->fbo[21].format	},
	{"fbo_22_width",	VTYPE_INT,			&GLDRV->fbo[22].width	},
	{"fbo_22_height",	VTYPE_INT,			&GLDRV->fbo[22].height	},
	{"fbo_22_format",	VTYPE_STRING,		&GLDRV->fbo[22].format	},
	{"fbo_23_width",	VTYPE_INT,			&GLDRV->fbo[23].width	},
	{"fbo_23_height",	VTYPE_INT,			&GLDRV->fbo[23].height	},
	{"fbo_23_format",	VTYPE_STRING,		&GLDRV->fbo[23].format	},
	{"fbo_24_width",	VTYPE_INT,			&GLDRV->fbo[24].width	},
	{"fbo_24_height",	VTYPE_INT,			&GLDRV->fbo[24].height	},
	{"fbo_24_format",	VTYPE_STRING,		&GLDRV->fbo[24].format	},
};

#define COMMANDS_NUMBER (sizeof(scriptCommand) / sizeof(tScriptCommand))

// ******************************************************************

// defined section commands
#define SECTION_IDENTIFIER		0
#define SECTION_START			1
#define SECTION_END				2
#define SECTION_LAYER			3
#define SECTION_BLEND			4
#define SECTION_BLEND_EQUATION	5
#define SECTION_ALPHA			6
#define SECTION_PARAM			7
#define SECTION_STRING			8
#define SECTION_UNIFORM			9
#define SECTION_SPLINE			10
#define SECTION_MODIFIER		11
#define SECTION_ENABLED			12

// defined section reserved keys
#define SECTION_COMMANDS_NUMBER 13

static char *scriptSectionCommand[SECTION_COMMANDS_NUMBER] = {
	"id", "start", "end", "layer", "blend", "blendequation",
	"alpha", "param", "string", "uniform", "spline", "modify", "enabled"
};

// ******************************************************************

#define BLEND_FUNC 15

glTable_t blendFunc[BLEND_FUNC] = {
	{ "ZERO",						GL_ZERO							},
	{ "ONE",						GL_ONE							},
	{ "DST_COLOR",					GL_DST_COLOR					},
	{ "ONE_MINUS_DST_COLOR",		GL_ONE_MINUS_DST_COLOR			},
	{ "SRC_ALPHA",					GL_SRC_ALPHA					},
	{ "ONE_MINUS_SRC_ALPHA",		GL_ONE_MINUS_SRC_ALPHA			},
	{ "DST_ALPHA",					GL_DST_ALPHA					},
	{ "ONE_MINUS_DST_ALPHA",		GL_ONE_MINUS_DST_ALPHA			},
	{ "CONSTANT_COLOR",				GL_CONSTANT_COLOR				},
	{ "ONE_MINUS_CONSTANT_COLOR",	GL_ONE_MINUS_CONSTANT_COLOR		},
	{ "CONSTANT_ALPHA",				GL_CONSTANT_ALPHA				},
	{ "ONE_MINUS_CONSTANT_ALPHA",	GL_ONE_MINUS_CONSTANT_ALPHA		},
	{ "SRC_ALPHA_SATURATE",			GL_SRC_ALPHA_SATURATE			},
	{ "SRC_COLOR",					GL_SRC_COLOR					},
	{ "ONE_MINUS_SRC_COLOR",		GL_ONE_MINUS_SRC_COLOR			},
};

// ******************************************************************

glTable_t blendEquationFunc[] = {
	{ "ADD",				GL_FUNC_ADD				},
	{ "SUBTRACT",			GL_FUNC_SUBTRACT		},
	{ "REVERSE_SUBTRACT",	GL_FUNC_REVERSE_SUBTRACT}
};

#define BLEND_EQUATION_FUNC (sizeof(blendEquationFunc) / sizeof(glTable_t))


// ******************************************************************

#define ALPHA_FUNC 8

glTable_t alphaFunc[ALPHA_FUNC] = {
	{ "NEVER",		GL_NEVER	},
	{ "LESS",		GL_LESS		},
	{ "EQUAL",		GL_EQUAL	},
	{ "LEQUAL",		GL_LEQUAL	},
	{ "GREATER",	GL_GREATER	},
	{ "NOTEQUAL",	GL_NOTEQUAL	},
	{ "GEQUAL",		GL_GEQUAL	},
	{ "ALWAYS",		GL_ALWAYS	}
};


demokernel * demokernel::getInstance() {
	if (m_pThis == NULL) {
		m_pThis = new demokernel();
	}
	return m_pThis;
}

demokernel::demokernel() {
	// initialize global kernel variables
	this->demoName = "Phoneix Spontz Demoengine";
//	this->debug = FALSE;
//#ifdef _DEBUG
	this->debug = TRUE;
//#endif
	this->record = FALSE;
	this->recordFps = 60.0f;
	this->loop = TRUE;
	this->sound = TRUE;
	this->bench = FALSE;
	this->startTime = 0.0f;
	this->endTime = 20.0f;
	this->slaveMode = FALSE;
	this->beat = 0.0f;
	this->beat_ratio = 1.4f;
	this->beat_fadeout = 4.0f;

	// Demo states
	this->drawFbo = 0;

}

void demokernel::getArguments(int argc, char *argv[]) {

	if (argc > 1) {
		dataFolder = argv[1];
	}
	else {
		dataFolder = "./data"; // Set the demo folder to the current project file (the "./" is not really required)
	}
}

void demokernel::initDemo() {
	
	// initialize graphics driver
	GLDRV->initGraphics();
	LOG->Info(LOG_HIGH,"OpenGL environment created");
	LOG->Info(LOG_MED, "OpenGL library version is: %s", glGetString(GL_VERSION));
	
	// initialize sound driver
	if (this->sound)
		BASSDRV->init();

	// Create the camera
	this->camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	// Create a basic light
	this->light = new Light();

	// Start loading Basic resources
	RES->loadAllResources();

	if (DEMO->slaveMode) {
		LOG->Info(LOG_HIGH, "Running in network slave mode");
		NETDRV->init(28000);	// Init network on port 28000
		LOG->Info(LOG_MED, "Network Dyad.c library version is: %s", NETDRV->getVersion());
	}
	else
		LOG->Info(LOG_HIGH, "Running in standalone mode");

	// initialize global control variables
	this->initControlVars();

	// prepare sections
	this->initSectionQueues();

	// get initial sync timer values
	this->initTimer();
}

void demokernel::mainLoop() {
	if (this->debug)
		LOG->Info(LOG_MED, "************ Main demo loop started!");

	this->state = DEMO_PLAY;

	/* Loop until the user closes the window */
	while ((!GLDRV->window_should_close()) && (!this->exitDemo)) {
		// Poll for and process events
		GLDRV->processInput();
		// 
		this->doExec();

		glfwPollEvents();
	}
}

void demokernel::doExec() {

	// TODO: Implement mouse events
	// capture mouse move and reset the position
	//demoSystem.mouseButtons = SDL_GetMouseState(&demoSystem.mouseX, &demoSystem.mouseY);
	
	// Save the mouse position differences from the center of the window
	//demoSystem.mouseXvar = (glDriver.width / 2) - demoSystem.mouseX;
	//demoSystem.mouseYvar = (glDriver.height / 2) - demoSystem.mouseY;

	// Reset the mouse position if the left mouse button is pressed
	/*if (demoSystem.mouseButtons & SDL_BUTTON(1)) {
		SDL_ShowCursor(FALSE);

		SDL_WarpMouseInWindow(glDriver.pSDLWindow, glDriver.width / 2, glDriver.height / 2);

	}
	else if (demoSystem.debug) {
		SDL_ShowCursor(TRUE);
	}
	*/

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

string demokernel::getFolder(string path)
{
	return (this->dataFolder+path);
}

void demokernel::load_spos() {
	struct _finddata_t FindData;
	intptr_t hFile;
	string fullpath;
	string ScriptRelativePath;
	fullpath = dataFolder + "/*.spo";
	LOG->Info(LOG_MED, "Scanning folder: %s", fullpath.c_str());
	if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
		do {
			ScriptRelativePath = dataFolder + "/" + FindData.name;
			LOG->Info(LOG_LOW, "Reading file: %s", ScriptRelativePath.c_str());
			load_spo(ScriptRelativePath);
			LOG->Info(LOG_LOW, "Finished loading file!");
		} while (_findnext(hFile, &FindData) == 0);
	}
	LOG->Info(LOG_MED,"Finished loading all files.");
}

bool demokernel::load_scriptFromNetwork(string sScript)
{
	Section *my_sec;
	int sec_id;
	sec_id = this->load_scriptData(sScript, "Network");
	if (sec_id >= 0)
		my_sec = this->sectionManager.section[sec_id];
	// Load the data from the section
	my_sec->loaded = my_sec->load();
	if (my_sec->loaded)
		LOG->Info(LOG_LOW, "  Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, my_sec->identifier.c_str(), my_sec->DataSource.c_str());
	else
		LOG->Error("  Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, my_sec->identifier.c_str(), my_sec->DataSource.c_str());

	return my_sec->loaded;
}

void demokernel::initTimer()
{
	this->beforeFrameTime = static_cast<float>(glfwGetTime());
}

void demokernel::calculateFPS(float frameTime)
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
	if (this->record) {
		this->realFrameTime = 1.0f / this->recordFps;

	}
	else {
		this->afterFrameTime = static_cast<float>(glfwGetTime());
		this->realFrameTime = this->afterFrameTime - this->beforeFrameTime;
		this->beforeFrameTime = this->afterFrameTime;
	}

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
	Section *ds = NULL;
	Section *ds_tmp = NULL;
	sLoading *ds_loading = NULL;
	float startTime = 0.0f;
	int i;
	int sec_id;

	// Set the demo state to loading
	this->state = DEMO_LOADING;
	LOG->Info(LOG_HIGH,"Loading Start...");

	if (this->debug) {
		startTime = (float)glfwGetTime();
	}
	
	// Search for the loading section, if not found, we will create one
	for (i = 0; i < this->sectionManager.section.size(); i++) {
		if (this->sectionManager.section[i]->type == SectionType::Loading)
			ds_loading = (sLoading*)this->sectionManager.section[i];
	}
	
	if (ds_loading == NULL) {
		LOG->Info(LOG_MED,"Loading section not found: using default loader");
		sec_id = this->sectionManager.addSection("loading", "Automatically created", TRUE);
		if (sec_id == SectionType::NOT_FOUND) {
			LOG->Error("Critical Error, Loading section not found and could not be created!");
			return;
		}
		else
			ds_loading = (sLoading*)this->sectionManager.section[sec_id];
	}
	// Demo states
	this->drawFps = 1;
	this->drawTiming = 1;
	this->drawSound = 0;
	this->drawFbo = 0;

	
	// preload, load and init loading section
	ds_loading->load();
	ds_loading->init();
	ds_loading->loaded = TRUE;
	ds_loading->inited = TRUE;
	ds_loading->exec();

	LOG->Info(LOG_MED,"  Loading section loaded, inited and executed for first time");

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

	LOG->Info(LOG_LOW,"  Ready Section queue complete: %d sections to be loaded", this->sectionManager.loadSection.size());

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
			LOG->Info(LOG_LOW, "  Section %d [id: %s, DataSource: %s] loaded OK!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());
		else
			LOG->Error("  Section %d [id: %s, DataSource: %s] not loaded properly!", sec_id, ds->identifier.c_str(), ds->DataSource.c_str());

		if (this->exitDemo) {
			this->closeDemo();
			exit(EXIT_SUCCESS);
		}
	}

	LOG->Info(LOG_MED,"Loading complete, %d sections have been loaded", this->loadedSections);
	
	// End loading
	ds_loading->end();
	ds_loading->ended = TRUE;
	
}

void demokernel::reInitSectionQueues() {
	Section *ds = NULL;
	int i;
	int sec_id;

	LOG->Info(LOG_LOW, "  Analysing sections that must be re-inited...");
	for (i = 0; i < this->sectionManager.execSection.size(); i++) {
		sec_id = this->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = this->sectionManager.section[sec_id];
		if ((ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {
			ds->inited = FALSE;			// Mark the section as not inited
			LOG->Info(LOG_LOW, "  Section %d [layer: %d id: %s] marked to be inited", sec_id, ds->layer, ds->identifier.c_str());
		}
	}
}

void demokernel::processSectionQueues() {
	Section *ds;
	int i;
	int sec_id;
	vector<Section*>::iterator it;


	LOG->Info(LOG_MED,"Start queue processing (end, init and exec) for second: %.4f", this->runTime);
	// We loop all the sections, searching for finished sections,
	// if any is found, we will remove from the queue and will execute the .end() function

	// Check the sections that need to be finalized
	LOG->Info(LOG_LOW,"  Analysing sections that can be removed...", this->runTime);
	for (it = this->sectionManager.section.begin(); it < this->sectionManager.section.end(); it++) {
		ds = *it;
		if ((ds->endTime <= this->runTime) && (ds->ended == FALSE)) {
			ds->end();
			ds->ended = TRUE;
			LOG->Info(LOG_LOW, "  Section [layer: %d id: %s type: %s] ended", ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}

	// Check the sections that need to be executed
	LOG->Info(LOG_LOW,"  Analysing sections that must be executed...", this->runTime);
	this->sectionManager.execSection.clear();
	for (i = 0; i< this->sectionManager.section.size(); i++) {
		ds = this->sectionManager.section[i];
		if ((ds->startTime <= this->runTime) && (ds->endTime >= this->runTime) &&		// If time is OK
			(ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {		// If its enabled, loaded and is not hte Loading section
			this->sectionManager.execSection.push_back(make_pair(ds->layer, i));		// Load the section: first the layer and then the ID
		}
	}
	sort(this->sectionManager.execSection.begin(), this->sectionManager.execSection.end());	// Sort sections by Layer

	LOG->Info(LOG_LOW,"  Exec Section queue complete: %d sections to be executed", this->sectionManager.execSection.size());
	// Run Init sections
	LOG->Info(LOG_LOW,"  Running Init Sections...");
	for (i = 0; i < this->sectionManager.execSection.size(); i++) {
		sec_id = this->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = this->sectionManager.section[sec_id];
		if (ds->inited == FALSE) {
			ds->runTime = DEMO->runTime - ds->startTime;
			ds->init();			// Init the Section
			ds->inited = TRUE;
			LOG->Info(LOG_LOW, "  Section %d [layer: %d id: %s type: %s] inited", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
	}



	// prepare engine for render
	GLDRV->initRender(true);
	
	// Run Exec sections
	LOG->Info(LOG_LOW,"  Running Exec Sections...");
	for (i = 0; i < this->sectionManager.execSection.size(); i++) {
		sec_id = this->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = this->sectionManager.section[sec_id];
		ds->runTime = DEMO->runTime - ds->startTime;
		ds->exec();			// Exec the Section
		LOG->Info(LOG_LOW, "  Section %d [layer: %d id: %s type: %s] executed", sec_id, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
	}

	LOG->Info(LOG_MED,"End queue processing!");

	if (this->debug) {
		if (this->drawFps)
			GLDRV->drawFps();
		if (this->drawTiming)
			GLDRV->drawTiming();
		if (this->drawFbo)
			GLDRV->drawFbo();
	}
	
	// swap buffer
	GLDRV->swap_buffers();
}

void demokernel::load_spo(string sFile) {
	string pScript;
	pScript = load_file(sFile);
	load_scriptData(pScript, sFile);
}

string demokernel::load_file(string sFile) {
	FILE * pFile;
	char *buffer;
	long lSize;
	size_t result;

	pFile = fopen(sFile.c_str(), "rb");
	if (pFile == NULL) {
		LOG->Error("demokernel::load_file: error opening file: '%s'", sFile.c_str());
		return 0;
	}

	// obtain file size
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);

	// allocate memory to contain the whole file:
	// we allocate one character more to ensure the returned string terminates in zero
	buffer = (char*)calloc(lSize + 1, sizeof(char));
	if (buffer == NULL) LOG->Error("demokernel::load_file: memory error in file: '%s'", sFile.c_str());

	// copy the file into the buffer:
	result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) LOG->Error("demokernel::load_file: read error in file: '%s'", sFile.c_str());

	// The whole file is now loaded in the memory buffer!

	// terminate
	fclose(pFile);
	return buffer;
}

int demokernel::load_scriptData(string sScript, string sFile) {
	const char*		name = "";
	char			line[256], key[512], value[512], tmp[512], tmp2[512];
	int				lineNum, com, i, values;
	float			fvalue;
	unsigned int	startPosition = 0;
	int				sec_id = -1;
	Section			*new_sec = NULL;
	Spline			*new_spl = NULL;

	// initialize script parse variables
	lineNum = 0;
	for (;;) {
		lineNum++;

		memset(line, '\0', 256);

		// get current script line
		// break if end of file

		// This function was returning startPosition > 0 when there weren't additional lines in the text variable, but only when
		// it was called in a very fast way (for example, loading two sections coming from the network very fast, one after another)
		// So we added a protection inside the function to return 0 if the startPosition is invalid (larger than the string length)
		startPosition = Util::getNextLine(sScript.c_str(), line, startPosition);

		// No more lines to process
		if (startPosition == 0)
			break;

		if (line[0] == '\0')
			continue;

		//LOG->Info("  Parsing line %i", lineNum);

		// Ignore comments or empty line
		if ((line[0] == ';') || (line[0] == '\n') || (line[0] == '\r')) {
			LOG->Info(LOG_LOW,"  Comments found or empty in line %i, ignoring this line.", lineNum);
			continue;
		}


		if (line[0] == '[') {
			// First we read the Section type (key)
			sscanf(line, "[%s]", key);
			key[strlen(key) - 1] = 0;

			// by default the section is enabled and marked as not loaded
			sec_id = -1;
			sec_id = this->sectionManager.addSection(key, "File: " + sFile, TRUE);
			if (sec_id != -1) {
				LOG->Info(LOG_LOW, "  Section %s added!", key);
				new_sec = this->sectionManager.section[sec_id];
			}
			else {
				LOG->Error("Section %s not supported! File skipped", key);
				return sec_id;
			}
		}
		// If we have already loaded the section type, then load it's parameters
		else if (sec_id != -1) {
			// Continue reading the Section body
			Util::getKeyValue(line, key, value);
			// Select local variable
			com = -1;
			for (i = 0; i < SECTION_COMMANDS_NUMBER; i++) {
				if (_strcmpi(key, scriptSectionCommand[i]) == 0) {
					com = i;
					break;
				}
			}
			// If the command is not recognized
			// look at the first character (fColor, cSpline, sTexture)
			if (com == -1) {
				switch (key[0]) {
				case 'f':
					com = SECTION_PARAM;
					break;
				case 's':
					com = SECTION_STRING;
					break;
				case 'u':
					com = SECTION_UNIFORM;
					break;
				case 'c':
					com = SECTION_SPLINE;
					break;
				case 'm':
					com = SECTION_MODIFIER;
					break;
				}
			}
			// If the command is correct, load the corresponding variable
			switch (com) {

			case SECTION_IDENTIFIER:
				values = sscanf(value, "%s", tmp);
				new_sec->identifier = tmp;
				LOG->Info(LOG_LOW, "  Section id: %s", new_sec->identifier.c_str());
				break;

			case SECTION_ENABLED:
				values = sscanf(value, "%i", &new_sec->enabled);
				LOG->Info(LOG_LOW, "  Section enabled state: %i", new_sec->enabled);
				break;

			case SECTION_START:
				values = sscanf(value, "%f", &new_sec->startTime);
				if (values != 1) LOG->Error("Invalid Start time in file %s, line: %s", sFile.c_str(), line);
				LOG->Info(LOG_LOW, "  Section Start time: %f", new_sec->startTime);
				break;

			case SECTION_END:
				values = sscanf(value, "%f", &new_sec->endTime);
				if (values != 1) LOG->Error("Invalid End time in file %s, line: %s", sFile.c_str(), line);
				LOG->Info(LOG_LOW, "  Section End time: %f", new_sec->endTime);
				new_sec->duration = new_sec->endTime - new_sec->startTime;
				if (new_sec->duration <= 0)
					LOG->Error("Section End time is less or equal than Start timeStart time!");
				break;

			case SECTION_LAYER:
				values = sscanf(value, "%i", &new_sec->layer);
				if (values != 1) LOG->Error("Invalid layer in file %s, line: %s", sFile.c_str(), line);
				LOG->Info(LOG_LOW, "  Section layer: %i", new_sec->layer);
				break;

			case SECTION_BLEND:
				values = sscanf(value, "%s %s", tmp, tmp2);
				if (values != 2) LOG->Error("Invalid blend format in file %s, line: %s", sFile.c_str(), line);

				new_sec->sfactor = getBlendCodeByName(tmp);
				new_sec->dfactor = getBlendCodeByName(tmp2);

				if ((new_sec->sfactor == -1) || (new_sec->dfactor == -1))
					LOG->Error("Invalid blend mode in file %s, line: %s", sFile.c_str(), line);

				new_sec->hasBlend = TRUE;
				LOG->Info(LOG_LOW, "  Section blend mode: source %i and destination %i", new_sec->sfactor, new_sec->dfactor);
				break;

			case SECTION_BLEND_EQUATION:
				values = sscanf(value, "%s", tmp);
				if (values != 1) LOG->Error("Invalid blend equation in file %s, line: %s", sFile.c_str(), line);
				new_sec->blendEquation = getBlendEquationCodeByName(tmp);
				LOG->Info(LOG_LOW, "  Section blend equation: %i", new_sec->blendEquation);
				break;

			case SECTION_ALPHA:
				values = sscanf(value, "%s %f %f", tmp, &new_sec->alpha1, &new_sec->alpha2);
				switch (values) {
				case 2:
					new_sec->alpha2 = new_sec->alpha1;
					break;
				case 3:
					break;
				default:
					LOG->Error("Invalid alpha");
					break;
				}
				new_sec->alphaFunc = getAlphaCodeByName(tmp);
				if (new_sec->alphaFunc == -1)
					LOG->Error("Invalid alpha function");
				new_sec->hasAlpha = TRUE;
				LOG->Info(LOG_LOW, "  Section alpha: from %f to %f", new_sec->alpha1, new_sec->alpha2);
				break;

			case SECTION_PARAM:
				fvalue = Util::getFloat(value);
				new_sec->param.push_back(fvalue);
				LOG->Info(LOG_LOW, "  Section parameter: %s = %f", key, fvalue);
				break;

			case SECTION_STRING:
				new_sec->strings.push_back(value);
				LOG->Info(LOG_LOW, "  Loaded string: \"%s\"", value);
				break;

			case SECTION_UNIFORM:
				new_sec->uniform.push_back(value);
				LOG->Info(LOG_LOW, "  Loaded uniform: \"%s\"", value);
				break;

			case SECTION_SPLINE:
				values = sscanf(value, "%s %f", tmp, &fvalue);

				switch (values) {
				case 1:
					fvalue = new_sec->duration;
					break;
				case 2:
					break;
				default:
					LOG->Error("Invalid spline");
					break;
				}
				new_spl = new Spline();
				new_spl->filename = DEMO->dataFolder + tmp;
				new_spl->duration = fvalue;
				new_sec->spline.push_back(new_spl);
				LOG->Info(LOG_LOW, "  Loaded Spline: %s", new_spl->filename.c_str());
				break;

			default:
				LOG->Error("Examined line: \"%s\" but unknown section variable (id: %d) was found.", line, com);
				break;
			}
		} else {
			// global variables
			Util::getKeyValue(line, key, value);

			// generic variable loading
			int *iptr;
			float *fptr;
			char **sptr;

			for (i = 0; i < COMMANDS_NUMBER; ++i) {
				if (_strcmpi(key, scriptCommand[i].cName) == 0) {
					switch (scriptCommand[i].vType) {
					case VTYPE_INT:
						sscanf(value, "%d", (int *)((char**)scriptCommand[i].vAddr));
						iptr = (int *)scriptCommand[i].vAddr;
						LOG->Info(LOG_LOW, "  Command found: %s = %d", scriptCommand[i].cName, *iptr);
						break;
					case VTYPE_FLOAT:
						sscanf(value, "%f", (float *)((char**)scriptCommand[i].vAddr));
						fptr = (float *)scriptCommand[i].vAddr;
						LOG->Info(LOG_LOW, "  Command found: %s = %f", scriptCommand[i].cName, *fptr);
						break;
					case VTYPE_STRING:
						*((char**)scriptCommand[i].vAddr) = _strdup(value);
						sptr = (char **)scriptCommand[i].vAddr;
						LOG->Info(LOG_LOW, "  Command found: %s = %s", scriptCommand[i].cName, *sptr);
						break;
					default:
						LOG->Error("%d is not a valid variable type id.", scriptCommand[i].vType);
						break;
					}
					break;
				}
		}

		if (i >= COMMANDS_NUMBER)
			LOG->Error("%s is not a valid SPO Script variable.", key);
		
		}
	}
	return sec_id;
}

int demokernel::getCodeByName(char *name, glTable_t *table, int size) {

	int i;

	for (i = 0; i < size; i++) {
		if (_strcmpi(name, table[i].name) == 0) {
			return table[i].code;
		}
	}

	return -1;
}

int demokernel::getBlendCodeByName(char *name) {
	return getCodeByName(name, blendFunc, BLEND_FUNC);
}

int demokernel::getBlendEquationCodeByName(char *name) {
	return getCodeByName(name, blendEquationFunc, BLEND_EQUATION_FUNC);
}

int demokernel::getAlphaCodeByName(char *name) {
	return getCodeByName(name, alphaFunc, ALPHA_FUNC);
}
