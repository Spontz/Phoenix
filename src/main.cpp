// main.cpp
// Spontz Demogroup

#define PHOENIX_MAIN

#include "main.h"

#include "debug/instrumentor.h"

int main(int argc, char* argv[]) {
	auto& demo = demokernel::GetInstance();
	auto& log = Logger::GetInstance();
	auto& gldrv = glDriver::GetInstance();

	demo.getArguments(argc, argv);

	// Load internal resources
	demo.allocateResources();

	// Define the Log level
	#ifdef _DEBUG
		log.setLogLevel(LogLevel::LOW); // Define the lowest log detail level
	#else
		log.setLogLevel(LogLevel::HIGH); // Define the highest log detail level
	#endif

	// Initialize the GL Framework
	gldrv.initFramework();

	// Check the data folder
	if (!demo.checkDataFolder()) {
		log.Error(("Critical error: Cannot find data folder in: " + demo.dataFolder + ", exit!").c_str());
		return 1;
	}

	log.Info(LogLevel::HIGH, "Phoenix Visuals Engine starting up: Let's make some noise!");

	PX_PROFILE_BEGIN_SESSION("DataLoad", "PhoenixProfile-DataLoad.json");
	log.Info(LogLevel::MED, "Loading Scripts...");
	if (!demo.load_config())
		return 0;
	demo.initNetwork(); // After loading config, we init the network
	demo.load_spos();
	PX_PROFILE_END_SESSION();

	log.Info(LogLevel::HIGH, "Initializing demo...");
	if (!demo.initDemo()) {
		log.CloseLogFile();
		return 0;
	}

	PX_PROFILE_BEGIN_SESSION("Runtime", "PhoenixProfile-Runtime.json");
	log.Info(LogLevel::HIGH, "Initializing main loop...");
	demo.mainLoop();
	PX_PROFILE_END_SESSION();

	log.Info(LogLevel::HIGH, "Closing demo. We hope you enjoyed watching this demo! See you next time! Watch more at www.spontz.org.");
	demo.closeDemo();
	log.CloseLogFile();

	return 0;
}
