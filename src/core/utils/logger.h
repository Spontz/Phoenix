// logger.h
// Spontz Demogroup

#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <string>

#define LOG_NONE 0
#define LOG_ERROR_ONLY 1
#define LOG_HIGH 2
#define LOG_MED 3
#define LOG_LOW 4

using namespace std;
#define LOG CLogger::GetLogger()

class CLogger {
public:
	char log_level;	// Log Level: LOG_HIGH, LOG_MED, LOG_LOW
	void Info(char level, const char * format, ...);
	void Error(const char * format, ...);

	static CLogger* GetLogger();
	void CloseLogFile();
private:
	CLogger();
	// Copy constructor for the Logger class.
	CLogger(const CLogger&) {};
	CLogger& operator=(const CLogger&) { return *this; };
	static const std::string m_sFileName;
	static CLogger* m_pThis;
	static ofstream m_Logfile;
};
#endif