// logger.h
// Spontz Demogroup

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>

enum class LoggerLevels : char {
	NONE = 0,
	ERROR_ONLY = 1,
	HIGH = 2,
	MED = 3,
	LOW = 4
};

// todo: remove, use enum class
char const LOG_NONE = 0;
char const LOG_ERROR_ONLY = 1;
char const LOG_HIGH = 2;
char const LOG_MED = 3;
char const LOG_LOW = 4;

class Logger {
public:
	static Logger& GetInstance();

private:
	Logger();

public:
	// hack: encapsulate
	char log_level_ = LOG_HIGH; // Log Level: LOG_HIGH, LOG_MED, LOG_LOW

public:
	void Info(char level, const char* format, ...) const;
	void Error(const char* format, ...) const;
	void CloseLogFile() const;

private:
	std::string output_file_ = "demo_log.txt";
	mutable std::ofstream log_ofstream_;
};

// hack: 0 macros
#define LOG (&Logger::GetInstance())

#endif
