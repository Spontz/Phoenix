// logger.cpp
// Spontz Demogroup

#include "main.h"
#include "logger.h"

Logger& Logger::GetInstance() {
	static Logger l;
	return l;
}

Logger::Logger()
	:
	m_netDriver_(netDriver::GetInstance()),
	m_outputFile_("demo_log.txt"),
	m_logLevel_(LogLevel::HIGH)
{
	if (DEMO->m_debug)
		OpenLogFile();
}

void Logger::setLogLevel(LogLevel level)
{
	if (level <= LogLevel::LOW)
		m_logLevel_ = level;
}

void Logger::Info(LogLevel level, const char* pszMessage, ...) const {
	if (DEMO->m_debug && this->m_logLevel_ >= level) {

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
		ss << "Info [" << std::put_time(timeinfo, "%T") << " t: " << DEMO->m_demoRunTime << "] " << pszText << std::endl;
		const auto strOutputString = ss.str();

		delete[] pszText;

		// Output to file
		m_ofstream_ << strOutputString;

		// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
		if (level <= LogLevel::HIGH)
			OutputDebugStringA(strOutputString.c_str());
#endif
	}
}

void Logger::SendEditor(const char* pszMessage, ...) const {
	// We send the message only if we are in debug mode and slave mode
	if (DEMO->m_debug == false || DEMO->slaveMode == 0)
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
	m_netDriver_.sendMessage(strOutputString);

	// Output to Visual Studio
#if defined(_DEBUG) && defined(WIN32)
	OutputDebugStringA(strOutputString.c_str());
#endif
}


void Logger::Error(const char* pszMessage, ...) const {
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
	m_ofstream_ << strOutputString;

	// Output to Visual Studio
	#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(strOutputString.c_str());
	#endif

	// Output to demo editor
	m_netDriver_.sendMessage("ERROR::" + strOutputString);
}

void Logger::OpenLogFile() const {
	if (!m_ofstream_.is_open())
		m_ofstream_.open(m_outputFile_.c_str(), std::ios::out | std::ios::trunc);
}

void Logger::CloseLogFile() const {
	if (m_ofstream_.is_open())
		m_ofstream_.close();
}
