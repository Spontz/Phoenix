// logger.cpp
// Spontz Demogroup

#include "logger.h"
#include "main.h"
#include "core/utils/utilities.h"

const string CLogger::m_sFileName = "demo_log.txt";
CLogger* CLogger::m_pThis = NULL;
ofstream CLogger::m_Logfile;


CLogger::CLogger() {
}

CLogger* CLogger::GetLogger() {
	if (m_pThis == NULL) {
		m_pThis = new CLogger();
		m_pThis->log_level = LOG_HIGH;
		if (DEMO->debug)
			m_Logfile.open(m_sFileName.c_str(), ios::out | ios::trunc);
	}
	return m_pThis;
}

void CLogger::CloseLogFile()
{
	if (DEMO->debug)
		m_Logfile.close();
}

#define STR_MAX_SIZE 2048

void CLogger::Info(char level, const char *message, ...) {
	va_list argptr;
	char text[STR_MAX_SIZE];
	char chain[STR_MAX_SIZE];
	// TODO use strings instead of arrays os fixed size to avoid errors!!!
	// write down the trace to the standard output
	if (DEMO->debug && this->log_level >= level) {
		va_start(argptr, message);
		vsprintf_s(text, message, argptr);
		va_end(argptr);
		sprintf_s(chain, "Info  [%.4f] %s\n", Util::CurrentTime(), text);
		m_Logfile << chain;
	}
}

void CLogger::Error(const char *message, ...) {
	va_list argptr;
	char text[STR_MAX_SIZE];
	char chain[STR_MAX_SIZE];
	// TODO use strings instead of arrays os fixed size to avoid errors!!!
	// write down the trace to the standard output
	if (DEMO->debug) {
		va_start(argptr, message);
		vsprintf_s(text, message, argptr);
		va_end(argptr);
		sprintf_s(chain, "Error [%.4f] %s\n", Util::CurrentTime(), text);
		m_Logfile << chain;
#ifdef _DEBUG
		OutputDebugStringA(chain);
#endif
		if (DEMO->slaveMode == 1) {
			string message = "ERROR::";
			message += chain;
			NETDRV->sendMessage(message);
		}

	}
}
