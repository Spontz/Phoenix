// main.cpp
// Spontz Demogroup

#define PHOENIX_MAIN

#include "main.h"

#include "debug/Instrumentor.h"

int main(int argc, char* argv[]) {
	auto& demo = demokernel::GetInstance();
	auto& gldrv = glDriver::GetInstance();

	demo.getArguments(argc, argv);

	// Load internal resources
	demo.allocateResources();

	// Define the Log level
	#ifdef _DEBUG
		Logger::setLogLevel(LogLevel::low); // Define the lowest log detail level
	#else
		Logger::setLogLevel(LogLevel::high); // Define the highest log detail level
	#endif

	// Initialize the GL Framework
	gldrv.initFramework();

	// Check the data folder
	if (!demo.checkDataFolder()) {
		Logger::error(("Critical error: Cannot find data folder in: " + demo.m_dataFolder + ", exit!").c_str());
		return 1;
	}

	Logger::info(LogLevel::high, "Phoenix Visuals Engine starting up: Let's make some noise!");

	PX_PROFILE_BEGIN_SESSION("DataLoad", "PhoenixProfile-DataLoad.json");
	Logger::info(LogLevel::med, "Loading Scripts...");
	if (!demo.load_config())
		return 0;
	demo.initNetwork(); // After loading config, we init the network
	demo.load_spos();
	PX_PROFILE_END_SESSION();

	Logger::info(LogLevel::high, "Initializing demo...");
	if (!demo.initDemo()) {
		Logger::closeLogFile();
		return 0;
	}

	PX_PROFILE_BEGIN_SESSION("Runtime", "PhoenixProfile-Runtime.json");
	Logger::info(LogLevel::high, "Initializing main loop...");
	demo.mainLoop();
	PX_PROFILE_END_SESSION();

	Logger::info(LogLevel::high, "Closing demo. We hope you enjoyed watching this demo! See you next time! Watch more at www.spontz.org.");
	demo.closeDemo();
	Logger::closeLogFile();

	return 0;
}
