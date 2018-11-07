// main.cpp
// Spontz Demogroup

#include <iostream>
//#include "core/demokernel.h"
#include "main.h"



int main(int argc, char *argv[]) {
	// Initialize the GL Framework
	GLDRV->initFramework();

	LOG->Info("Phoenix Visuals Engine starting up: Let's make some noise!");
	// Get "demo" folder path
	DEMO->getArguments(argc, argv);

	LOG->Info("Loading Scripts");
	DEMO->load_spos();
	
	if (DEMO->slaveMode)
		LOG->Info("Running in network slave mode");
	else
		LOG->Info("Running in standalone mode");


	LOG->Info("Initializing demo!");
	DEMO->initDemo();
	LOG->Info("Initializing main loop!");
	DEMO->mainLoop();
	LOG->Info("Closing demo. We hope you enjoyed watching this demo! See you next time! Watch more at www.spontz.org");
	DEMO->closeDemo();

	return 0;
}
