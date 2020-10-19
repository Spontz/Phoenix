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
	if (DEMO->debug)
		OpenLogFile();
}

void Logger::setLogLevel(LogLevel level)
{
	if (level <= LogLevel::LOW)
		m_logLevel_ = level;
}

void Logger::Info(LogLevel level, const char* message, ...) const {
	// write down the trace to the standard output
	if (DEMO->debug && this->m_logLevel_ >= level) {
		va_list argptr;
		char* Text_;	// Formatted text
		char* Chain_;	// Text chain to be written to file
		int iLen;
		const char* const cPrefix = "Info [%.4f] %s\n";
		double dTime = Util::CurrentTime();

		va_start(argptr, message);
		iLen = vsnprintf(NULL, 0, message, argptr);
		iLen++;
		Text_ = new char[iLen];
		vsnprintf(Text_, iLen, message, argptr);
		va_end(argptr);
		// Get the new size of the string
		iLen = snprintf(NULL, 0, cPrefix, dTime, Text_);
		iLen++;
		Chain_ = new char[iLen];
		snprintf(Chain_, iLen, cPrefix, dTime, Text_);
		m_ofstream_ << Chain_;
#if defined(_DEBUG) && defined(WIN32)
		if (level <= LogLevel::HIGH)
			OutputDebugStringA(Chain_);
#endif
		delete[] Text_;
		delete[] Chain_;
	}
}

void Logger::SendEditor(const char* message, ...) const {
	// write down the trace to the standard output
	if (DEMO->debug) {
		va_list argptr;
		char* Text_;	// Formatted text
		char* Chain_;	// Text chain to be written to file
		int iLen;
		const char* const cPrefix = "Message [%.4f] %s\n";
		double dTime = Util::CurrentTime();

		va_start(argptr, message);
		iLen = vsnprintf(NULL, 0, message, argptr);
		iLen++;
		Text_ = new char[iLen];
		vsnprintf(Text_, iLen, message, argptr);
		va_end(argptr);
		// Get the new size of the string
		iLen = snprintf(NULL, 0, cPrefix, dTime, Text_);
		iLen++;
		Chain_ = new char[iLen];
		snprintf(Chain_, iLen, cPrefix, dTime, Text_);
		m_ofstream_ << Chain_;
#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(Chain_);
#endif
		if (DEMO->slaveMode == 1) {
			std::string message = "INFO::";
			message += Chain_;
			m_netDriver_.sendMessage(message);
		}
		delete[] Text_;
		delete[] Chain_;
	}
}


void Logger::Error(const char* pszMessage, ...) const {
	if (!DEMO->debug)
		return;

	va_list argptr;
	va_start(argptr, pszMessage);
	const auto iLen = vsnprintf(NULL, 0, pszMessage, argptr) + 1;
	auto pszText = new char[iLen];
	vsnprintf(pszText, iLen, pszMessage, argptr);
	va_end(argptr);

	const std::time_t t = std::time(nullptr);
	char szDateTime[128];
	std::strftime(szDateTime, sizeof(szDateTime), "%F %T", std::gmtime(&t));

	std::stringstream ss;
	ss << "[" << szDateTime << "] Error: " << pszText << std::endl;
	const auto strOutputString = ss.str();

	delete[] pszText;

	// Output to file
	m_ofstream_ << strOutputString;

	// Output to Visual Studio
	#if defined(_DEBUG) && defined(WIN32)
		OutputDebugStringA(strOutputString.c_str());
	#endif

	// Output to demo editor
	if (DEMO->slaveMode == 1)
		m_netDriver_.sendMessage(std::string("ERROR::") + strOutputString);
}

void Logger::OpenLogFile() const {
	if (!m_ofstream_.is_open())
		m_ofstream_.open(m_outputFile_.c_str(), std::ios::out | std::ios::trunc);
}

void Logger::CloseLogFile() const {
	if (m_ofstream_.is_open())
		m_ofstream_.close();
}
