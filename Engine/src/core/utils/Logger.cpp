// Logger.cpp
// Spontz Demogroup

#include "main.h"
#include "core/utils/LoggerDeclarations.h"
#include "core/drivers/NetDriver.h"

#include <iomanip>

namespace Phoenix {

	constexpr std::string_view kOutputFile("demo_log.txt");
	std::ofstream Logger::kOutputStream;
	uint32_t kIndent = 0;

	LogLevel Logger::kLogLevel = LogLevel::high;

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

	std::string Logger::formatMsg(const std::string_view src, const std::string_view Message)
	{
		std::stringstream ss;
		// Indent
		for (uint32_t i = 0; i < kIndent; ++i)
			ss << "  ";

		ss << src << " " << Message << std::endl;
		return  ss.str();
	}

	void Logger::openLogFile()
	{
		if (!kOutputStream.is_open())
			kOutputStream.open(kOutputFile.data(), std::ios::out | std::ios::trunc);
	}

	void Logger::closeLogFile()
	{
		if (kOutputStream.is_open()) {
			kOutputStream.flush();
			kOutputStream.close();
		}
	}
}
