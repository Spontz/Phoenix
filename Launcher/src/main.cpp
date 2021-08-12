// Phoenix main launcher
// Spontz Demogroup

#define PHOENIX_MAIN

#include "Engine/src/main.h"

// #define PROFILE_PHOENIX

#ifdef PROFILE_PHOENIX
	#include "Engine/src/debug/Instrumentor.h"
#endif

int main(int argc, char* argv[]) {
	{
		auto& demo = *DEMO;
		auto& gldrv = *GLDRV;

		demo.getArguments(argc, argv);

		// Load internal resources
		demo.allocateResources();

		// Define the Log level
		Phoenix::Logger::setLogLevel(
#ifdef _DEBUG
			Phoenix::LogLevel::low // Define the lowest log detail level
#else
			Phoenix::LogLevel::high // Define the highest log detail level
#endif
		);

		// Initialize the GL Framework
		gldrv.initFramework();

		// Check the data folder
		if (!demo.checkDataFolder()) {
			Phoenix::Logger::error(("Critical error: Cannot find data folder in: " + demo.m_dataFolder + ", exit!").c_str());
			return 1;
		}

		Phoenix::Logger::info(Phoenix::LogLevel::high, "Phoenix Visuals Engine starting up: Let's make some noise!");

		//PX_PROFILE_BEGIN_SESSION("DataLoad", "PhoenixProfile-DataLoad.json");
		Phoenix::Logger::info(Phoenix::LogLevel::med, "Loading Scripts...");
		if (!demo.loadSpoConfig())
			return 0;
		demo.initNetwork(); // After loading config, we init the network
		demo.loadSpoFiles();
		//PX_PROFILE_END_SESSION();

		Phoenix::Logger::info(Phoenix::LogLevel::high, "Initializing demo...");
		if (!demo.initDemo()) {
			Phoenix::Logger::closeLogFile();
			return 0;
		}

#ifdef PROFILE_PHOENIX
		PX_PROFILE_BEGIN_SESSION("Runtime", "PhoenixProfile-Runtime.json");
#endif

		Phoenix::Logger::info(Phoenix::LogLevel::high, "Initializing main loop...");
		demo.mainLoop();

#ifdef PROFILE_PHOENIX
		PX_PROFILE_END_SESSION();
#endif

		Phoenix::Logger::info(Phoenix::LogLevel::high, "Closing demo. We hope you enjoyed watching this demo! See you next time! Watch more at www.spontz.org.");
		demo.closeDemo();
		Phoenix::Logger::closeLogFile();
	}

	Phoenix::glDriver::release();
	Phoenix::demokernel::release();
	Phoenix::NetDriver::release();

	return 0;
}
