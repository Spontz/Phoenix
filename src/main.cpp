// main.cpp
// Spontz Demogroup

#include <iostream>
//#include "core/demokernel.h"
#include "main.h"



int main(int argc, char *argv[]) {
	
	
	LOG->Info("Demokernel variables inited to default state");

	
	DEMO->getArguments(argc, argv);

	LOG->Info("Initializing demo!");
	DEMO->initDemo();
	LOG->Info("Initializing main loop!");
	DEMO->mainLoop();
	LOG->Info("Closing demo :(");
	DEMO->closeDemo();

	return 0;
}
