// logger.h
// Spontz Demogroup

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>

enum class LoggerLevels : char {
	NONE = 0,
	HIGH = 1,
	MED = 2,
	LOW = 3
};

// todo: remove, use enum class
char const LOG_NONE = 0;
char const LOG_HIGH = 1;
char const LOG_MED = 2;
char const LOG_LOW = 3;

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
	void OpenLogFile() const;
	void CloseLogFile() const;

private:
	std::string output_file_ = "demo_log.txt";
	mutable std::ofstream log_ofstream_;
};

// hack: 0 macros
#define LOG (&Logger::GetInstance())

#endif
