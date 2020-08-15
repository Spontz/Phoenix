// logger.h
// Spontz Demogroup

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>


enum LogLevel : char const {
	NONE = 0,
	HIGH = 1,
	MED = 2,
	LOW = 3
};

class Logger {
public:
	static Logger& GetInstance();

private:
	Logger();

public:
	void setLogLevel(const LogLevel level);
	void Info(const LogLevel level, const char* message, ...) const;
	void SendEditor(const char* message, ...) const;
	void Error(const char* message, ...) const;
	void OpenLogFile() const;
	void CloseLogFile() const;

private:
	LogLevel				log_level_;
	std::string				output_file_ = "demo_log.txt";
	mutable std::ofstream	log_ofstream_;
	netDriver&				m_netDriver;
};

// hack: 0 macros
#define LOG (&Logger::GetInstance())

#endif
