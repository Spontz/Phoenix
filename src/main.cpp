// main.cpp
// Spontz Demogroup

#include <iostream>
#include "main.h"



int main(int argc, char *argv[]) {
	// Define the Log level
	LOG->log_level = LOG_LOW; // Define the log with the lowest detail

	// Initialize the GL Framework
	GLDRV->initFramework();
	LOG->Info(LOG_HIGH, "Phoenix Visuals Engine starting up: Let's make some noise!");
	
	// Get "demo" folder path
	DEMO->getArguments(argc, argv);

	LOG->Info(LOG_MED, "Loading Scripts");
	DEMO->load_spos();
	
	if (DEMO->slaveMode)
		LOG->Info(LOG_MED, "Running in network slave mode");
	else
		LOG->Info(LOG_MED, "Running in standalone mode");


	LOG->Info(LOG_HIGH, "Initializing demo!");
	DEMO->initDemo();
	LOG->Info(LOG_HIGH, "Initializing main loop!");
	DEMO->mainLoop();
	LOG->Info(LOG_HIGH, "Closing demo. We hope you enjoyed watching this demo! See you next time! Watch more at www.spontz.org");
	DEMO->closeDemo();
	LOG->CloseLogFile();

	return 0;
}
