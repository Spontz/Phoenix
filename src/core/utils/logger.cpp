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

void CLogger::Info(char level, const char *message, ...) {
	va_list argptr;
	char text[1024];
	char chain[1024];

	// write down the trace to the standard output
	if (DEMO->debug && this->log_level >= level) {
		va_start(argptr, message);
		vsprintf_s(text, message, argptr);
		va_end(argptr);
		sprintf_s(chain, "Info  [%s] %s", Util::CurrentTime().c_str(), text);
		m_Logfile << chain << "\n";
#ifdef WIN32
		OutputDebugStringA(chain);
		if (text[0] != '\0' || text[strlen(text) - 1] != '\n')
			OutputDebugStringA("\n");
#endif
	}
}

void CLogger::Error(const char *message, ...) {
	va_list argptr;
	char text[1024];
	char chain[1024];

	// write down the trace to the standard output
	if (DEMO->debug) {
		va_start(argptr, message);
		vsprintf_s(text, message, argptr);
		va_end(argptr);
		sprintf_s(chain, "Error [%s] %s", Util::CurrentTime().c_str(), text);
		m_Logfile << chain << "\n";
		//}
#ifdef WIN32
		OutputDebugStringA(chain);
		if (text[0] != '\0' || text[strlen(text) - 1] != '\n')
			OutputDebugStringA("\n");
#endif
	}
}

CLogger& CLogger::operator<<(const string& sMessage) {
//	m_Logfile << "\n" << Util::CurrentTime() << ":\t";
//	m_Logfile << sMessage << "\n";
//	cout << "\n" << Util::CurrentTime() << ":\t";
//	cout << sMessage << "\n";
	return *this;
}