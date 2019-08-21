// main.cpp
// Spontz Demogroup

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "main.h"



int main(int argc, char *argv[]) {
	DEMO->getArguments(argc, argv);
	
	// Define the Log level
	LOG->log_level_ = LOG_HIGH; // Define the highest log detail level
	#ifdef _DEBUG
	LOG->log_level_ = LOG_LOW; // Define the lowest log detail level
	#endif
	
	// Initialize the GL Framework
	GLDRV->initFramework();
	
	// Check the data folder
	if (!DEMO->checkDataFolder()) {
		LOG->Error("Critical error: Cannot find data folder in: %s, exit!", DEMO->dataFolder.c_str());
		exit(0);
	}

	LOG->Info(LOG_HIGH, "Phoenix Visuals Engine starting up: Let's make some noise!");


	LOG->Info(LOG_MED, "Loading Scripts");
	DEMO->load_spos();
	
	LOG->Info(LOG_HIGH, "Initializing demo!");
	DEMO->initDemo();
	LOG->Info(LOG_HIGH, "Initializing main loop!");
	DEMO->mainLoop();
	LOG->Info(LOG_LOW, "Average Framerate: %.2f", (float)DEMO->frameCount/DEMO->runTime);
	LOG->Info(LOG_HIGH, "Closing demo. We hope you enjoyed watching this demo! See you next time! Watch more at www.spontz.org");
	DEMO->closeDemo();
	LOG->CloseLogFile();

	return 0;
}
