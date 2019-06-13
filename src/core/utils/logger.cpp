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


void CLogger::Info(char level, const char *message, ...) {
	va_list argptr;
	
	// write down the trace to the standard output
	if (DEMO->debug && this->log_level >= level) {
		va_start(argptr, message);
		vsnprintf(text, STR_MAX_SIZE, message, argptr);
		va_end(argptr);
		// Get the new size of the string
		snprintf(chain, STR_MAX_SIZE, "Info  [%.4f] %s", Util::CurrentTime(), text);
		m_Logfile << chain << "\n";
	}
}

void CLogger::Error(const char *message, ...) {
	va_list argptr;

	// write down the trace to the standard output
	if (DEMO->debug) {
		va_start(argptr, message);
		vsnprintf(text, STR_MAX_SIZE, message, argptr);
		va_end(argptr);
		snprintf(chain, STR_MAX_SIZE, "Error [%.4f] %s", Util::CurrentTime(), text);
		m_Logfile << chain << "\n";
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
