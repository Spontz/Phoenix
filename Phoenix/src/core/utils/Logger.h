// Logger.h
// Spontz Demogroup

#pragma once

#include "core/drivers/NetDriver.h"
#include <fstream>

namespace Phoenix {

	enum class LogLevel {
		none = 0,
		high = 1,
		med = 2,
		low = 3
	};

	class Logger {
	public:
		static void setLogLevel(LogLevel level);
		static void info(LogLevel level, const char* pszMessage, ...);
		static void sendEditor(const char* pszMessage, ...);
		static void error(const char* pszMessage, ...);
		static void openLogFile();
		static void closeLogFile();

	private:
		static std::ofstream m_ofstream;
		static LogLevel m_bLogLevel;
		static const std::string m_strOutputFile;
		static const NetDriver& m_netDriver;
	};
}