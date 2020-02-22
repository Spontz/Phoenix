// main.cpp
// Spontz Demogroup

#define PHOENIX_MAIN

#include "main.h"

int main(int argc, char* argv[]) {
	auto& demo = demokernel::GetInstance();
	auto& log = Logger::GetInstance();
	auto& gldrv = glDriver::GetInstance();

	demo.getArguments(argc, argv);

	// Define the Log level
	#ifdef _DEBUG
		log.log_level_ = LOG_LOW; // Define the lowest log detail level
	#else
		log.log_level_ = LOG_HIGH; // Define the highest log detail level
	#endif

	// Initialize the GL Framework
	gldrv.initFramework();

	// Check the data folder
	if (!demo.checkDataFolder()) {
		log.Error(("Critical error: Cannot find data folder in: " + demo.dataFolder + ", exit!").c_str());
		return 1;
	}

	log.Info(LOG_HIGH, "Phoenix Visuals Engine starting up: Let's make some noise!");

	log.Info(LOG_MED, "Loading Scripts...");
	if (!demo.load_config())
		return 0;
	demo.load_spos();

	log.Info(LOG_HIGH, "Initializing demo...");
	demo.initDemo();

	log.Info(LOG_HIGH, "Initializing main loop...");
	demo.mainLoop();

	log.Info(LOG_HIGH, "Closing demo. We hope you enjoyed watching this demo! See you next time! Watch more at www.spontz.org.");
	demo.closeDemo();
	log.CloseLogFile();

	return 0;
}
