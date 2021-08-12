// Logger.cpp
// Spontz Demogroup

#include "main.h"
#include "core/utils/Logger.h"

#include <iomanip>

namespace Phoenix {

	const NetDriver& Logger::m_netDriver{ NetDriver::GetInstance() };
	const std::string Logger::m_strOutputFile{ "demo_log.txt" };
	LogLevel Logger::m_bLogLevel{ LogLevel::high };
	std::ofstream Logger::m_ofstream;

	void Logger::setLogLevel(LogLevel level)
	{
		static bool bInitialized = false;

		if (!bInitialized) {
			if (DEMO->m_debug)
				openLogFile();
			bInitialized = true;
		}

		if (level <= LogLevel::low)
			m_bLogLevel = level;
	}

	void Logger::info(LogLevel level, const char* pszMessage, ...) {
		if (DEMO->m_debug && m_bLogLevel >= level) {

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
			ss << "Info [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << pszText << std::endl;
			const auto strOutputString = ss.str();

			delete[] pszText;

			// Output to file
			m_ofstream << strOutputString;

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
		ss << "INFO::Info [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << pszText << std::endl;
		const auto strOutputString = ss.str();

		delete[] pszText;

		// Output to editor
		m_netDriver.sendMessage(strOutputString);

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
		ss << "Error [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << pszText << std::endl;
		const auto strOutputString = ss.str();

		delete[] pszText;

		// Output to file
		m_ofstream << strOutputString;

		// Output to screen log
		GLDRV->guiAddLog(strOutputString);

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(strOutputString.c_str());
#endif

		// Output to demo editor
		m_netDriver.sendMessage("ERROR::" + strOutputString);
	}

	void Logger::openLogFile() {
		if (!m_ofstream.is_open())
			m_ofstream.open(m_strOutputFile.c_str(), std::ios::out | std::ios::trunc);
	}

	void Logger::closeLogFile() {
		if (m_ofstream.is_open())
			m_ofstream.close();
	}
}