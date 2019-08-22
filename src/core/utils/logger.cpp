// logger.cpp
// Spontz Demogroup

#include "main.h"
#include "logger.h"

#define HACK_LOGGER_STR_MAX_SIZE 2048

Logger & Logger::Instance() {
	static Logger l;
	return l;
}

Logger::Logger() {
	if (DEMO->debug)
		log_ofstream_.open(output_file_.c_str(), ios::out | ios::trunc);
}

void Logger::CloseLogFile() const {
	if (DEMO->debug)
		log_ofstream_.close();
}

void Logger::Info(char level, const char* message, ...) const {
	va_list argptr;
	char Text_[HACK_LOGGER_STR_MAX_SIZE];	// Formatted text
	char Chain_[HACK_LOGGER_STR_MAX_SIZE];	// Text chain to be written to file

	// write down the trace to the standard output
	if (DEMO->debug && this->log_level_ >= level) {
		va_start(argptr, message);
		vsnprintf(Text_, HACK_LOGGER_STR_MAX_SIZE, message, argptr);
		va_end(argptr);
		// Get the new size of the string
		snprintf(Chain_, HACK_LOGGER_STR_MAX_SIZE, "Info  [%.4f] %s\n", Util::CurrentTime(), Text_);
		log_ofstream_ << Chain_;
	}
}

void Logger::Error(const char* message, ...) const {
	va_list argptr;
	char Text_[HACK_LOGGER_STR_MAX_SIZE];	// Formatted text
	char Chain_[HACK_LOGGER_STR_MAX_SIZE];	// Text chain to be written to file

	// write down the trace to the standard output
	if (DEMO->debug) {
		va_start(argptr, message);
		vsnprintf(Text_, HACK_LOGGER_STR_MAX_SIZE, message, argptr);
		va_end(argptr);
		snprintf(Chain_, HACK_LOGGER_STR_MAX_SIZE, "Error [%.4f] %s\n", Util::CurrentTime(), Text_);
		log_ofstream_ << Chain_;
#ifdef _DEBUG
		OutputDebugStringA(Chain_);
#endif
		if (DEMO->slaveMode == 1) {
			string message = "ERROR::";
			message += Chain_;
			NETDRV->sendMessage(message);
		}

	}
}
