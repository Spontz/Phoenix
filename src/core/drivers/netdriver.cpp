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
	m_iPortSend(28001),
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
		m_iPortSend
	);

	m_pServConnect_ = dyad_newStream();
	dyad_addListener(m_pServConnect_, DYAD_EVENT_CONNECT, dyadOnConnect, nullptr);
	dyad_connect(m_pServConnect_, "127.0.0.1", m_iPortSend);
}

void netDriver::update() const
{
	dyad_update();
}

const char* netDriver::getVersion() const
{
	return dyad_getVersion();
}

//char* netDriver::processMessage(const char* pszMessage) const
std::string netDriver::processMessage(const std::string& pszMessage) const
{
	// Outcoming information
	std::string pszResult;			// Result of the operation
	std::string pszInfo;			// Information message
	std::string pszResponse = "";	// Final response message

	const std::vector<std::string> Message = splitMessage(pszMessage);

	const std::string sIdentifier = Message[0];
	const std::string sType = Message[1];
	const std::string sAction = Message[2];
	
	pszResult = "OK";
	

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
			float time = std::stof(Message[3]);
			DEMO->setStartTime(time);
		}
		else if (sAction == "currentTime") {
			float time = std::stof(Message[3]);
			DEMO->setCurrentTime(time);
		}
		else if (sAction == "endTime") {
			float time = std::stof(Message[3]);
			DEMO->setEndTime(time);
		}
		else if (sAction == "ping") {
		}
		else if (sAction == "end") {
			DEMO->exitDemo = true;
		}
		else {
			pszResult = "NOK";
			pszInfo = "Unknown command (" + sAction + "): " + pszMessage;
		}
	}
	else if (sType == "section") {
		// Sections processing
		if (sAction == "new") {
			if (DEMO->load_scriptFromNetwork(Message[3]) != 0) {
				pszResult = "NOK";
				pszInfo = "Section load failed";
			}
		}
		else if (sAction == "toggle") {
			pszResult = "NOK";
			pszInfo = "toggle command (" + sAction + "): " + pszMessage;
			//DEMO->sectionManager.toggleSection(Message[3]);
		}
		else if (sAction == "delete") {
			DEMO->sectionManager.deleteSection(Message[3]);
		}
		else if (sAction == "update") {
			DEMO->sectionManager.updateSection(Message[3],Message[4]);
		}
		else if (sAction == "setStartTime") {
			DEMO->sectionManager.setSectionsStartTime(Message[3], Message[4]);
		}
		else if (sAction == "setEndTime") {
			DEMO->sectionManager.setSectionsEndTime(Message[3], Message[4]);
		}
		else if (sAction == "setLayer") {
			DEMO->sectionManager.setSectionLayer(Message[3], Message[4]);
		}
		else {
			pszResult = "NOK";
			pszInfo = "Unknown section command (" + sAction + "): " + pszMessage;
		}
	}
	else {
		pszResult = "NOK";
		pszInfo = "Unknown network message type (" + sType + "): " + pszMessage;
	}

	// Create response
	pszResponse = sIdentifier + kDelimiterChar +
		pszResult + kDelimiterChar +
		std::to_string(DEMO->fps) + kDelimiterChar +
		std::to_string(DEMO->state) + kDelimiterChar +
		std::to_string(DEMO->demo_runTime) + kDelimiterChar +
		pszInfo;
	
	LOG->Info(LogLevel::LOW, "Sending response: [%s]", pszResponse.c_str());

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
		static_cast<int>(message.size())
	);
}


// private static //////////////////////////////////////////////////////////////////////////////////

void netDriver::dyadOnData(dyad_Event* const pDyadEvent) {
	const auto pszResponse = netDriver::GetInstance().processMessage(pDyadEvent->data);

	// Send the response and close the connection
	dyad_write(pDyadEvent->stream, pszResponse.c_str(), static_cast<int>(pszResponse.size()));
	dyad_end(pDyadEvent->stream);
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

std::vector<std::string> netDriver::splitMessage(const std::string& message) const
{
	std::vector<std::string> strings;
	std::istringstream f(message);
	std::string s;
	// Split the string by spaces
	while (std::getline(f, s, kDelimiterChar)) {
		strings.push_back(s);
	}
	return strings;
}
