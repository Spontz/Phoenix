// main.cpp
// Spontz Demogroup

#include <iostream>
#include "main.h"



int main(int argc, char *argv[]) {
	DEMO->getArguments(argc, argv);
	
	// Define the Log level
	LOG->log_level = LOG_HIGH; // Define the highest log detail level
	#ifdef _DEBUG
	LOG->log_level = LOG_LOW; // Define the lowest log detail level
	#endif

	// Initialize the GL Framework
	GLDRV->initFramework();
	
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
