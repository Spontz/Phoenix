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
	// Logs an info message (@param format string for the message to be logged)
	void Info(char level, const char * format, ...);
	// Logs an error message (@param format string for the message to be logged)
	void Error(const char * format, ...);

	// << overloaded function to Logs a message (@param sMessage message to be logged)
	CLogger& operator<<(const string& sMessage);
	// Funtion to create the instance of logger class (@return singleton object of Clogger class..)
	static CLogger* GetLogger();
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