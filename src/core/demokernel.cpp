// demoKernel.cpp
// Spontz Demogroup

#include <iostream>
#include <io.h>
#include <Windows.h>
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
	{"gl_zbuffer",		VTYPE_INT,			&GLDRV->zbuffer			},
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
#define SECTION_IDENTIFIER	0
#define SECTION_START		1
#define SECTION_END			2
#define SECTION_LAYER		3
#define SECTION_BLEND		4
#define SECTION_ALPHA		5
#define SECTION_PARAM		6
#define SECTION_STRING		7
#define SECTION_SPLINE		8
#define SECTION_MODIFIER	9
#define SECTION_ENABLED    10

// defined section reserved keys
#define SECTION_COMMANDS_NUMBER 11

static char *scriptSectionCommand[SECTION_COMMANDS_NUMBER] = {
	"id", "start", "end", "layer", "blend", "alpha",
	"param", "string", "spline", "modify", "enabled"
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

// ******************************************************************

typedef struct {
	char *name;
	int tex_iformat;		// internalformat
	int tex_format;
	int tex_type;
} glTexTable_t;

// ******************************************************************

glTexTable_t textureModes[] = {
	{ "RGB",			GL_RGB8,				GL_RGB,				GL_UNSIGNED_BYTE },
	{ "RGBA",			GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE },
	{ "RGB_16F",		GL_RGB16F,				GL_RGB,				GL_FLOAT },
	{ "RGBA_16F",		GL_RGBA16F,				GL_RGBA,			GL_FLOAT },
	{ "RGB_32F",		GL_RGB32F,				GL_RGB,				GL_FLOAT },
	{ "RGBA_32F",		GL_RGBA32F,				GL_RGBA,			GL_FLOAT },
};
#define TEXTURE_MODE (sizeof(textureModes) / sizeof(glTexTable_t))

// ******************************************************************



demokernel * demokernel::getInstance() {
	if (m_pThis == NULL) {
		m_pThis = new demokernel();
	}
	return m_pThis;
}

demokernel::demokernel() {
	// initialize global kernel variables
	this->demoName = "Phoneix Spontz Demoengine";
	this->debug = TRUE;
	this->record = FALSE;
	this->recordFps = 60.0f;
	this->loop = TRUE;
	this->sound = TRUE;
	this->bench = FALSE;
	this->startTime = 0.0f;
	this->endTime = 20.0f;
	this->slaveMode = FALSE;
	this->beat_ratio = 1.4f;
	this->beat_fadeout = 4.0f;
}

void demokernel::getArguments(int argc, char *argv[]) {

	if (argc > 1) {
		demoDir = argv[1];
	}
	else {
		demoDir = "./data"; // Set the demo folder to the current project file
		/*char *lastSlash;
		int chars;
		lastSlash = strrchr(argv[0], '\\');
		if (lastSlash == NULL) {
			demoDir = ".";
		}
		else {
			chars = (int)strlen(argv[0]) - (int)strlen(lastSlash);
			demoDir = (char *)malloc(chars + 1);
			strncpy(demoDir, argv[0], chars);
			demoDir[chars] = 0;
		}*/
	}
}

void demokernel::initDemo() {
	
	// initialize graphics driver
	GLDRV->init();
	LOG->Info("OpenGL environment created");
	
	// initialize sound driver
	if (this->sound)
		BASSDRV->init();


	//TODO: Gestionar el section manager mejor
	//this->sectionManager.init();

	/*
	// TODO: Implement network management
	if (demoSystem.slaveMode) {
		// initialize network driver
		network_init();
	}
	*/

	// initialize global control variables
	this->initControlVars();

	// prepare sections
	this->initSectionQueues();

	/*

	

	// initialize debugging font
	text_load_font(&debugFont, strDebugFont, 16, 16, "data/fonts/font.tga");

	// get initial sync timer values
	init_timer();
*/
}

void demokernel::mainLoop() {
	//BASS_Update(200);

	float startTime = static_cast<float>(glfwGetTime());
	float newTime = 0.0f;
	float gameTime = 0.0f;

	/* Loop until the user closes the window */
	while (!GLDRV->window_should_close())
	{
		/* Update game time value */
		newTime = static_cast<float>(glfwGetTime());
		gameTime = newTime - startTime;

		/* Render here */
		GLDRV->render(gameTime);

		/* Swap front and back buffers */
		GLDRV->swap_buffers();

		/* Poll for and process events */
		glfwPollEvents();
	}
}

void demokernel::closeDemo() {
	GLDRV->close();
}

void demokernel::load_spos() {
	struct _finddata_t FindData;
	intptr_t hFile;
	string demoFolder = this->demoDir;
	string fullpath;
	string ScriptRelativePath;
	fullpath = demoFolder + "/*.spo";
	LOG->Info("Scanning folder: %s", fullpath.c_str());
	if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
		do {
			ScriptRelativePath = demoFolder + "/" + FindData.name;
			LOG->Info("Reading file: %s", ScriptRelativePath.c_str());
			load_spo(ScriptRelativePath);
			//spoReader *sporead = new spoReader();
			//loadScriptFile(ScriptRelativePath);
			LOG->Info("Finished loading file!!");
		} while (_findnext(hFile, &FindData) == 0);
	}
	LOG->Info("Finished loading all files.");
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
	char isLast;

	// Set the demo state to loading
	this->state = DEMO_LOADING;
	LOG->Info("Loading Start...");

	if (this->debug) {
		startTime = (float)glfwGetTime();
	}
	
	// Search for the loading section, if not found, we will create one
	for (i = 0; i < this->sectionManager.section.size(); i++) {
		if (this->sectionManager.section[i]->type == SectionType::Loading)
			ds_loading = (sLoading*)this->sectionManager.section[i];
	}
	
	if (ds_loading == NULL) {
		LOG->Info("Loading section not found: using default loader");
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

	// Section accounting
	this->numSections = 0;
	this->numReadySections = 0;
	this->loadedSections = 0;
	
	// preload, load and init loading section
	ds_loading->preload();
	ds_loading->load();
	ds_loading->init();
	ds_loading->exec();

	// reset section queues. TODO: Not needed right?
	//demoSystem.readySection = NULL;
	//demoSystem.runSection = NULL;

	LOG->Info("Loading: section setup complete, %.2f seconds\n", ((float)glfwGetTime() - startTime));

	for (i = 0; i < this->sectionManager.section.size(); i++) {
		ds = this->sectionManager.section[i];
		if ((DEMO->slaveMode == 1) || (((ds->startTime < DEMO->endTime) || fabs(DEMO->endTime) < FLT_EPSILON) && (ds->endTime > DEMO->startTime))) {
			DEMO->numReadySections++;

			// TODO: SPLINES Support
			// load section splines (to avoid code load in the sections)
			//loadSplines(ds);

			// first list element
			if (!DEMO->readySection) {
				demoSystem.readySection = ds;

				// ordered insert on ready list
			}
			else {
				ds_tmp = demoSystem.readySection;
				isLast = FALSE;
				while ((!isLast) && (ds_tmp->startTime < ds->startTime)) {
					if (ds_tmp->nextRdy) ds_tmp = (tDemoSection*)ds_tmp->nextRdy;
					else isLast = TRUE;
				}

				if (isLast) {
					ds_tmp->nextRdy = ds;
					ds->priorRdy = ds_tmp;
				}
				else {
					if (ds_tmp->priorRdy) (*(tDemoSection*)ds_tmp->priorRdy).nextRdy = ds;
					else demoSystem.readySection = ds;
					ds->priorRdy = ds_tmp->priorRdy;
					ds->nextRdy = ds_tmp;
					ds_tmp->priorRdy = ds;
				}
			}
		}
		//ds = (tDemoSection*)ds->next;
		
	}

	/*
	// view all sections looking for ready sections
	ds = demoSystem.demoSection;
	while (ds != NULL) {

		if (    (  demoSystem.slaveMode == 1  ) || (  ( (ds->startTime < demoSystem.endTime) || fabs(demoSystem.endTime) < FLT_EPSILON ) && ( ds->endTime > demoSystem.startTime )  )    )
			{
			demoSystem.numReadySections++;

			// load section splines (to avoid code load in the sections)
			loadSplines(ds);

			// first list element
			if (!demoSystem.readySection) {
				demoSystem.readySection = ds;

			// ordered insert on ready list
			} else {
				ds_tmp = demoSystem.readySection;
				isLast = FALSE;
				while ((!isLast) && (ds_tmp->startTime < ds->startTime)) {
					if (ds_tmp->nextRdy) ds_tmp = (tDemoSection*) ds_tmp->nextRdy;
					else isLast = TRUE;
				}

				if (isLast) {
					ds_tmp->nextRdy = ds;
					ds->priorRdy = ds_tmp;
				} else {
					if (ds_tmp->priorRdy) (*(tDemoSection*)ds_tmp->priorRdy).nextRdy = ds;
					else demoSystem.readySection = ds;
					ds->priorRdy = ds_tmp->priorRdy;
					ds->nextRdy = ds_tmp;
					ds_tmp->priorRdy = ds;
				}
			}
		}
		ds = (tDemoSection*) ds->next;
	}

	// all sections will be preloaded except loading section
	demoSystem.numSections += SECTIONS_NUMBER-1;

#ifdef _DEBUG
	dkernel_trace("Loading: build ready queue, %.2f seconds\n", 0.001f * ((float) SDL_GetTicks() - startTime));
	dkernel_trace("Loading: %d sections to be preloaded\n", demoSystem.numSections);
#endif

	// preload all used demo sections
	for (i = 1; i < SECTIONS_NUMBER; ++i) {
		sectionFunction[i].preload();
		++demoSystem.loadedSections;

		// update loading
		loading->exec();

		// event handler
		if (SDL_PollEvent(&event)) eventHandler(event);

		if (demoSystem.exitDemo) {
			dkernel_closeDemo();
			exit(EXIT_SUCCESS);
		}
	}

	#ifdef _DEBUG
		dkernel_trace("Loading: sections preloaded! %.2f seconds", 0.001f * ((float) SDL_GetTicks() - startTime));
	#endif	

	#ifdef _DEBUG
		dkernel_trace("Running Load(.load()) functions... %d sections to be loaded", demoSystem.numReadySections);
	#endif	
	// load all ready sections
	ds = demoSystem.readySection;
	while (ds != NULL) {
		#ifdef _DEBUG
		dkernel_trace(" Section Loaded: %s", sectionFunction[ds->staticSectionIndex].scriptName);
		#endif

		// section load
		mySection = ds;
		sectionFunction[ds->staticSectionIndex].load();
		demoSystem.loadedSections++;

		ds = ds->nextRdy;

		// update loading
		mySection = loadingSection;
		loading->exec();

		// event handler
		if (SDL_PollEvent(&event)) eventHandler(event);

		if (demoSystem.exitDemo) {
			dkernel_closeDemo();
			exit(EXIT_SUCCESS);
		}
	}

#ifdef _DEBUG
	dkernel_trace("Loading: sections loaded, %.2f seconds\n", 0.001f * ((float)SDL_GetTicks() - startTime));
#endif		

	// end loading
	loading->end();
	
	*/

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

void demokernel::load_scriptData(string sScript, string sFile) {
	const char*		name = "";
	char			line[256], key[512], value[512], tmp[512], tmp2[512];
	int				lineNum, com, i, values;
	float			fvalue;
	unsigned int	startPosition = 0;
	int				sec_id = -1;
	Section			*new_sec = NULL;

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
			LOG->Info("  Comments found or empty in line %i, ignoring this line.", lineNum);
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
				LOG->Info("  Section %s added!", key);
				new_sec = this->sectionManager.section[sec_id];
			}
			else {
				LOG->Error("Section %s not supported! File skipped", key);
				return;
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
				LOG->Info("  Section id: %s", new_sec->identifier.c_str());
				break;

			case SECTION_ENABLED:
				values = sscanf(value, "%i", &new_sec->enabled);
				LOG->Info("  Section enabled state: %i", new_sec->enabled);
				break;

			case SECTION_START:
				values = sscanf(value, "%f", &new_sec->startTime);
				if (values != 1) LOG->Error("Invalid Start time in file %s, line: %s", sFile.c_str(), line);
				LOG->Info("  Section Start time: %f", new_sec->startTime);
				break;

			case SECTION_END:
				values = sscanf(value, "%f", &new_sec->endTime);
				if (values != 1) LOG->Error("Invalid End time in file %s, line: %s", sFile.c_str(), line);
				LOG->Info("  Section End time: %f", new_sec->endTime);
				new_sec->duration = new_sec->endTime - new_sec->startTime;
				if (new_sec->duration <= 0)
					LOG->Error("Section End time is less or equal than Start timeStart time!");
				break;

			case SECTION_LAYER:
				values = sscanf(value, "%i", &new_sec->layer);
				if (values != 1) LOG->Error("Invalid layer in file %s, line: %s", sFile.c_str(), line);
				LOG->Info("  Section layer: %i", new_sec->layer);
				break;

			case SECTION_BLEND:
				values = sscanf(value, "%s %s", tmp, tmp2);
				if (values != 2) LOG->Error("Invalid blend format in file %s, line: %s", sFile.c_str(), line);

				new_sec->sfactor = getBlendCodeByName(tmp);
				new_sec->dfactor = getBlendCodeByName(tmp2);

				if ((new_sec->sfactor == -1) || (new_sec->dfactor == -1))
					LOG->Error("Invalid blend mode in file %s, line: %s", sFile.c_str(), line);

				new_sec->hasBlend = TRUE;
				LOG->Info("  Section blend mode: source %i and destination %i", new_sec->sfactor, new_sec->dfactor);
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
				LOG->Info("  Section alpha: from %f to %f", new_sec->alpha1, new_sec->alpha2);
				break;

			case SECTION_PARAM:
				if (new_sec->paramNum >= SECTION_PARAMS)
					LOG->Error("Too many parameters, engine is limited to %i parameters", SECTION_PARAMS);

				values = Util::getFloatVector(value, &new_sec->param[new_sec->paramNum], SECTION_PARAMS);
				new_sec->paramNum += values;

				LOG->Info("  Section parameter: %s = %f (%i found)", key, new_sec->param[new_sec->paramNum - 1], values);
				break;

			case SECTION_STRING:
				if (new_sec->stringNum >= SECTION_STRINGS)
					LOG->Error("Too many strings, engine is limited to %i strings", SECTION_STRINGS);

				new_sec->strings[new_sec->stringNum] = _strdup(value);
				new_sec->stringNum++;

				LOG->Info("  Loaded string: \"%s\"", value);
				break;

			case SECTION_SPLINE:
				if (new_sec->splineNum >= SECTION_SPLINES)
					LOG->Error("Too many splines, engine is limited to %i splines", SECTION_SPLINES);

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
				new_sec->splineFiles[new_sec->splineNum] = _strdup(tmp);
				new_sec->splineDuration[new_sec->splineNum] = fvalue;
				new_sec->splineNum++;
				LOG->Info("  Loaded Spline");
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
						LOG->Info("  Command found: %s = %d", scriptCommand[i].cName, *iptr);
						break;
					case VTYPE_FLOAT:
						sscanf(value, "%f", (float *)((char**)scriptCommand[i].vAddr));
						fptr = (float *)scriptCommand[i].vAddr;
						LOG->Info("  Command found: %s = %f", scriptCommand[i].cName, *fptr);
						break;
					case VTYPE_STRING:
						*((char**)scriptCommand[i].vAddr) = _strdup(value);
						sptr = (char **)scriptCommand[i].vAddr;
						LOG->Info("  Command found: %s = %s", scriptCommand[i].cName, *sptr);
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

int demokernel::getAlphaCodeByName(char *name) {
	return getCodeByName(name, alphaFunc, ALPHA_FUNC);
}

int demokernel::getRenderModeByName(char *name) {
	//return getCodeByName(name, renderMode, RENDER_MODE);
	return -1;
}

int demokernel::getTextureFormatByName(char *name) {
	//return getTexFormatByName(name, textureModes, TEXTURE_MODE);
	return -1;
}

int demokernel::getTextureInternalFormatByName(char *name) {
	//return getTexInternalFormatByName(name, textureModes, TEXTURE_MODE);
	return -1;
}

int demokernel::getTextureTypeByName(char *name) {
	//return getTexTypeByName(name, textureModes, TEXTURE_MODE);
	return -1;
}

int demokernel::getSectionByName(char * name) {
	/*	int i;

		// get section index
		for (i = 0; i < SECTIONS_NUMBER; i++) {
			if (_strcmpi(name, sectionFunction[i].scriptName) == 0) {
				return i;
			}
		}
		*/
	return -1;
}