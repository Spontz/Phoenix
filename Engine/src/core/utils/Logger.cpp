// Logger.cpp
// Spontz Demogroup

#include "main.h"
#include "core/utils/Logger.h"
#include "core/drivers/NetDriver.h"

#include <iomanip>

namespace Phoenix {

	constexpr std::string_view kOutputFile("demo_log.txt");
	LogLevel kLogLevel = LogLevel::high;
	std::ofstream kOutputStream;
	uint32_t kIndent = 0;


	std::string formatMsg(const std::string_view src, const std::string_view Message)
	{
		const std::time_t t = std::time(nullptr);
		auto timeinfo = localtime(&t);

		// Indent
		std::stringstream ss;
		for (uint32_t i = 0; i < kIndent; ++i)
			ss << "  ";

		ss << src << " [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << Message << std::endl;
		return ss.str();

	}


	Logger::ScopedIndent::ScopedIndent()
	{
		++kIndent;
	}

	Logger::ScopedIndent::~ScopedIndent()
	{
		--kIndent;
	}

	void Logger::setLogLevel(LogLevel level)
	{
		static bool bInitialized = false;

		if (!bInitialized) {
			if (DEMO->m_debug)
				openLogFile();
			bInitialized = true;
		}

		if (level <= LogLevel::low)
			kLogLevel = level;
	}

	void Logger::info(LogLevel level, const std::string_view Message)
	{
		if (!DEMO->m_debug || kLogLevel < level)
			return;

		// Output to file
		const auto s = formatMsg("Info", Message);
		kOutputStream << s;

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		if (level <= LogLevel::high)
			OutputDebugStringA(s.c_str());
#endif
	}

	void Logger::sendEditor(const std::string_view Message)
	{
		// We send the message only if we are in debug mode and slave mode
		if (!DEMO->m_debug || !DEMO->m_slaveMode)
			return;

		const auto s = formatMsg("INFO::Info", Message);

		// Output to editor
		NetDriver::getInstance().sendMessage(s);

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(s.c_str());
#endif
	}

	void Logger::error(const std::string_view Message)
	{
		if (!DEMO->m_debug)
			return;

		const auto s = formatMsg("Error", Message);

		// Output to file
		kOutputStream << s;

		// Output to screen log
		GLDRV->guiAddLog(s);

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(s.c_str());
#endif

		// Output to demo editor
		NetDriver::getInstance().sendMessage("ERROR::" + s);
	}

	void Logger::openLogFile()
	{
		if (!kOutputStream.is_open())
			kOutputStream.open(kOutputFile.data(), std::ios::out | std::ios::trunc);
	}

	void Logger::closeLogFile()
	{
		if (kOutputStream.is_open())
			kOutputStream.close();
	}
}
