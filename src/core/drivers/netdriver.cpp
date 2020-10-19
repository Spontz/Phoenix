// netdriver.cpp
// Spontz Demogroup

// includes ////////////////////////////////////////////////////////////////////////////////////////

#include "netdriver.h"

#include <core/demokernel.h>


// globals /////////////////////////////////////////////////////////////////////////////////////////

auto const kDelimiterChar = '\x1f';
auto const kDelimiterString = "\x1f";


// public static ///////////////////////////////////////////////////////////////////////////////////

netDriver& netDriver::GetInstance() {
	static netDriver obj;
	return obj;
}


// constructors/destructor /////////////////////////////////////////////////////////////////////////

netDriver::netDriver()
	:
	m_iPortReceive(28000),
	m_iPortSend_(28001),
	m_bInitialized_(false),
	m_bConnectedToEditor_(false),
	m_pServConnect_(nullptr)
{
}

netDriver::~netDriver()
{
	dyad_shutdown();
}


// public

void netDriver::init()
{
	dyad_init();

	dyad_Stream* serv = dyad_newStream();
	//dyad_setNoDelay(serv, 1); // Disable Nagle's algorithm
	dyad_setUpdateTimeout(0);	// Disable waiting
	// Listeners for answering responses from the editor
	dyad_addListener(serv, DYAD_EVENT_ERROR, dyadOnError, nullptr);
	dyad_addListener(serv, DYAD_EVENT_ACCEPT, dyadOnAccept, nullptr);
	dyad_addListener(serv, DYAD_EVENT_LISTEN, dyadOnListen, nullptr);
	dyad_listenEx(serv, "0.0.0.0", m_iPortReceive, 511);

	connectToEditor();

	m_bInitialized_ = true;
}

void netDriver::connectToEditor()
{
	// Listener for sending messages to the editor
	LOG->Info(
		LogLevel::MED,
		"Network: outgoing messages will be done through port: %d",
		m_iPortSend_
	);

	m_pServConnect_ = dyad_newStream();
	dyad_addListener(m_pServConnect_, DYAD_EVENT_CONNECT, dyadOnConnect, nullptr);
	dyad_connect(m_pServConnect_, "127.0.0.1", m_iPortSend_);
}

void netDriver::update() const
{
	dyad_update();
}

const char* netDriver::getVersion() const
{
	return dyad_getVersion();
}

char* netDriver::processMessage(const char* pszMessage) const
{
	// Outcoming information
	auto pszResponse = new char[4096];
	char* pszResult;

	auto const sIdentifier = getParamString(pszMessage, 1);
	auto const sType = getParamString(pszMessage, 2);
	auto const sAction = getParamString(pszMessage, 3);

	pszResult = "OK";
	char pszInfo[1024];

	LOG->Info(
		LogLevel::LOW,
		"Message received: [identifier: %s] [type: %s] [action: %s]",
		sIdentifier.c_str(),
		sType.c_str(),
		sAction.c_str()
	);

	if (sType == "command") {
		// Commands processing
		if (sAction == "pause") {
			DEMO->pauseDemo();
		}
		else if (sAction == "play") {
			DEMO->playDemo();
		}
		else if (sAction == "restart") {
			DEMO->restartDemo();
		}
		else if (sAction == "startTime") {
			DEMO->setStartTime(getParamFloat(pszMessage, 4));
		}
		else if (sAction == "currentTime") {
			DEMO->setCurrentTime(getParamFloat(pszMessage, 4));
		}
		else if (sAction == "endTime") {
			DEMO->setEndTime(getParamFloat(pszMessage, 4));
		}
		else if (sAction == "ping") {
		}
		else if (sAction == "end") {
			DEMO->exitDemo = true;
		}
		else {
			pszResult = "NOK";
			sprintf(pszInfo, "Unknown command (%s)", pszMessage);
		}
	}
	else if (sType == "section") {
		// Sections processing
		if (sAction == "new") {
			if (DEMO->load_scriptFromNetwork(getParamString(pszMessage, 4)) != 0) {
				pszResult = "NOK";
				sprintf(pszInfo, "Section load failed");
			}
		}
		else if (sAction == "toggle") {
			DEMO->sectionManager.toggleSection(getParamString(pszMessage, 4));
		}
		else if (sAction == "delete") {
			DEMO->sectionManager.deleteSection(getParamString(pszMessage, 4));
		}
		else if (sAction == "update") {
			DEMO->sectionManager.updateSection(
				getParamString(pszMessage, 4),
				getParamString(pszMessage, 5)
			);
		}
		else if (sAction == "setStartTime") {
			DEMO->sectionManager.setSectionsStartTime(
				getParamString(pszMessage, 4),
				getParamString(pszMessage, 5)
			);
		}
		else if (sAction == "setEndTime") {
			DEMO->sectionManager.setSectionsEndTime(
				getParamString(pszMessage, 4),
				getParamString(pszMessage, 5)
			);
		}
		else if (sAction == "setLayer") {
			DEMO->sectionManager.setSectionLayer(
				getParamString(pszMessage, 4),
				getParamString(pszMessage, 5)
			);
		}
		else {
			pszResult = "NOK";
			sprintf(pszInfo, "Unknown section referenced in network message (%s)", pszMessage);
		}
	}
	else {
		pszResult = "NOK";
		sprintf(pszInfo, "Unknown network message type (%s)", pszMessage);
	}

	// Create response
	sprintf(
		pszResponse, "%s%c%s%c%f%c%d%c%f%c%s",

		sIdentifier.c_str(), kDelimiterChar,
		pszResult, kDelimiterChar,
		DEMO->fps, kDelimiterChar,
		DEMO->state, kDelimiterChar,
		DEMO->demo_runTime, kDelimiterChar,
		pszInfo
	);

	LOG->Info(LogLevel::LOW, "Sending response: [%s]", pszResponse);

	// return response (to be deleted later)
	return pszResponse;
}

void netDriver::sendMessage(std::string const& message) const
{
	if (!m_bConnectedToEditor_)
		return;

	dyad_write(
		m_pServConnect_,
		message.c_str(),
		static_cast<int>(message.length())
	);
}


// private static //////////////////////////////////////////////////////////////////////////////////

void netDriver::dyadOnData(dyad_Event* const pDyadEvent) {
	const auto pszResponse = netDriver::GetInstance().processMessage(pDyadEvent->data);

	// Send the response and close the connection
	dyad_write(pDyadEvent->stream, pszResponse, int(strlen(pszResponse)));
	dyad_end(pDyadEvent->stream);

	delete[] pszResponse;
}

void netDriver::dyadOnAccept(dyad_Event* const pDyadEvent)
{
	dyad_addListener(pDyadEvent->remote, DYAD_EVENT_DATA, dyadOnData, nullptr);
}

void netDriver::dyadOnListen(dyad_Event* const pDyadEvent)
{
	LOG->Info(
		LogLevel::MED,
		"Network listener started in port: %d",
		dyad_getPort(pDyadEvent->stream)
	);
}

void netDriver::dyadOnError(dyad_Event* const pDyadEvent)
{
	LOG->Error("Network server error: %s", pDyadEvent->msg);
}

void netDriver::dyadOnConnect(dyad_Event* const pDyadEvent)
{
	netDriver::GetInstance().m_bConnectedToEditor_ = true;

	LOG->Info(
		LogLevel::MED,
		"Network: Connected to editor through port: %d",
		dyad_getPort(pDyadEvent->stream)
	);
}


// private /////////////////////////////////////////////////////////////////////////////////////////

std::string netDriver::getParamString(const char* pszMessage, int32_t iRequestedParameter) const
{
	int32_t iCounter = 1;
	auto pszParameter = _strdup(pszMessage);

	for (
		pszParameter = strtok(pszParameter, kDelimiterString);
		iCounter < iRequestedParameter;
		pszParameter = strtok(nullptr, kDelimiterString)
		)
		++iCounter;

	std::string r(pszParameter);
	delete[] pszParameter;
	return r;
}

float netDriver::getParamFloat(const char* pszMessage, int32_t iRequestedParameter) const
{
	return std::stof(getParamString(pszMessage, iRequestedParameter));
}
