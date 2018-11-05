// logger.h
// Spontz Demogroup

#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <string>
using namespace std;
#define LOG CLogger::GetLogger()

class CLogger {
public:
	// Logs an info message (@param format string for the message to be logged)
	void Info(const char * format, ...);
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