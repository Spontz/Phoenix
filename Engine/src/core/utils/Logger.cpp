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

	void Logger::info(LogLevel level, const char* pszMessage, ...) {
		if (DEMO->m_debug && kLogLevel >= level) {

			// Get the message
			va_list argptr;
			va_start(argptr, pszMessage);
			const auto iLen = vsnprintf(NULL, 0, pszMessage, argptr) + 1;
			auto pszText = new char[iLen];
			vsnprintf(pszText, iLen, pszMessage, argptr);
			va_end(argptr);

			// Get the time
			const std::time_t t = std::time(nullptr);
			auto timeinfo = localtime(&t);

			std::stringstream ss;
			for (uint32_t i = 0; i < kIndent; ++i)
				ss << "  ";
			ss << "Info [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << pszText << std::endl;
			const auto strOutputString = ss.str();

			delete[] pszText;

			// Output to file
			kOutputStream << strOutputString;

			// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
			if (level <= LogLevel::high)
				OutputDebugStringA(strOutputString.c_str());
#endif
		}
	}

	void Logger::sendEditor(const char* pszMessage, ...) {
		// We send the message only if we are in debug mode and slave mode
		if (!DEMO->m_debug || !DEMO->m_slaveMode)
			return;

		// Get the message
		va_list argptr;
		va_start(argptr, pszMessage);
		const auto iLen = vsnprintf(NULL, 0, pszMessage, argptr) + 1;
		auto pszText = new char[iLen];
		vsnprintf(pszText, iLen, pszMessage, argptr);
		va_end(argptr);

		// Get the time
		const std::time_t t = std::time(nullptr);
		struct tm* timeinfo = localtime(&t);

		std::stringstream ss;
		for (uint32_t i = 0; i < kIndent; ++i)
			ss << "  ";
		ss << "INFO::Info [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << pszText << std::endl;
		const auto strOutputString = ss.str();

		delete[] pszText;

		// Output to editor
		NetDriver::getInstance().sendMessage(strOutputString);

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(strOutputString.c_str());
#endif
	}

	void Logger::error(const char* pszMessage, ...) {
		if (!DEMO->m_debug)
			return;

		// Get the message
		va_list argptr;
		va_start(argptr, pszMessage);
		const auto iLen = vsnprintf(NULL, 0, pszMessage, argptr) + 1;
		auto pszText = new char[iLen];
		vsnprintf(pszText, iLen, pszMessage, argptr);
		va_end(argptr);

		// Get the time
		const std::time_t t = std::time(nullptr);
		struct tm* timeinfo = localtime(&t);

		std::stringstream ss;
		for (uint32_t i = 0; i < kIndent; ++i)
			ss << "  ";
		ss << "Error [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << pszText << std::endl;
		const auto strOutputString = ss.str();

		delete[] pszText;

		// Output to file
		kOutputStream << strOutputString;

		// Output to screen log
		GLDRV->guiAddLog(strOutputString);

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(strOutputString.c_str());
#endif

		// Output to demo editor
		NetDriver::getInstance().sendMessage("ERROR::" + strOutputString);
	}

	void Logger::openLogFile() {
		if (!kOutputStream.is_open())
			kOutputStream.open(kOutputFile.data(), std::ios::out | std::ios::trunc);
	}

	void Logger::closeLogFile() {
		if (kOutputStream.is_open())
			kOutputStream.close();
	}
}