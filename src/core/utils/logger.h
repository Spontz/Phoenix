// logger.h
// Spontz Demogroup

#pragma once

#include "core/drivers/netdriver.h"

#include <string>
#include <fstream>
#include <iomanip>

enum class LogLevel {
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
	void setLogLevel(LogLevel level);

public:
	void Info(LogLevel level, const char* message, ...) const;
	void SendEditor(const char* message, ...) const;
	void Error(const char* message, ...) const;
	void OpenLogFile() const;
	void CloseLogFile() const;

private:
	mutable std::ofstream	m_ofstream_;

private:
	LogLevel				m_logLevel_;
	const std::string		m_outputFile_;
	const netDriver&		m_netDriver_;
};

// HACK: 0 macros
#define LOG (&Logger::GetInstance())
