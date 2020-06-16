// logger.cpp
// Spontz Demogroup

#include "main.h"
#include "logger.h"

Logger & Logger::GetInstance() {
	static Logger l;
	return l;
}

Logger::Logger() {
	log_level_ = LogLevel::HIGH;
	if (DEMO->debug)
		OpenLogFile();
}

void Logger::setLogLevel(const LogLevel level)
{
	if (level <= LogLevel::LOW)
		log_level_ = level;
}

void Logger::Info(const LogLevel level, const char* message, ...) const {
	// write down the trace to the standard output
	if (DEMO->debug && this->log_level_ >= level) {
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
		log_ofstream_ << Chain_;
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
		log_ofstream_ << Chain_;
#ifdef _DEBUG
		OutputDebugStringA(Chain_);
#endif
		if (DEMO->slaveMode == 1) {
			std::string message = "INFO::";
			message += Chain_;
			NETDRV->sendMessage(message);
		}
		delete[] Text_;
		delete[] Chain_;
	}
}

void Logger::Error(const char* message, ...) const {
	// write down the trace to the standard output
	if (DEMO->debug) {
		va_list argptr;
		char* Text_;	// Formatted text
		char* Chain_;	// Text chain to be written to file
		int iLen;
		const char* const cPrefix = "Error [%.4f] %s\n";
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
		log_ofstream_ << Chain_;
#ifdef _DEBUG
		OutputDebugStringA(Chain_);
#endif
		if (DEMO->slaveMode == 1) {
			std::string message = "ERROR::";
			message += Chain_;
			NETDRV->sendMessage(message);
		}
		delete[] Text_;
		delete[] Chain_;
	}
}

void Logger::OpenLogFile() const {
	if (!log_ofstream_.is_open())
		log_ofstream_.open(output_file_.c_str(), std::ios::out | std::ios::trunc);
}

void Logger::CloseLogFile() const {
	if (log_ofstream_.is_open())
		log_ofstream_.close();
}
