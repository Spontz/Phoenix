// NetDriver.cpp
// Spontz Demogroup

// includes ////////////////////////////////////////////////////////////////////////////////////////

#include "core/drivers/NetDriver.h"
#include "core/demokernel.h"

namespace Phoenix {


	// globals /////////////////////////////////////////////////////////////////////////////////////////

	auto const kDelimiterChar = '\x1f';
	auto const kDelimiterString = "\x1f";


	// public static ///////////////////////////////////////////////////////////////////////////////////

	NetDriver& NetDriver::GetInstance() {
		static NetDriver obj;
		return obj;
	}


	// constructors/destructor /////////////////////////////////////////////////////////////////////////

	NetDriver::NetDriver()
		:
		m_iPortReceive(28000),
		m_iPortSend(28001),
		m_bInitialized_(false),
		m_bConnectedToEditor_(false),
		m_pServConnect_(nullptr)
	{
	}

	NetDriver::~NetDriver()
	{
		dyad_shutdown();
	}


	// public

	void NetDriver::init()
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

	void NetDriver::connectToEditor()
	{
		// Listener for sending messages to the editor
		Logger::info(
			LogLevel::med,
			"Network: outgoing messages will be done through port: %d",
			m_iPortSend
		);

		m_pServConnect_ = dyad_newStream();
		dyad_addListener(m_pServConnect_, DYAD_EVENT_CONNECT, dyadOnConnect, nullptr);
		dyad_connect(m_pServConnect_, "127.0.0.1", m_iPortSend);
	}

	void NetDriver::update() const
	{
		dyad_update();
	}

	const char* NetDriver::getVersion() const
	{
		return dyad_getVersion();
	}

	std::string NetDriver::processMessage(const std::string& sMessage) const
	{
		// Outcoming information
		std::string sResult = "OK";		// Result of the operation
		std::string sInfo = "";			// Information message
		std::string sResponse = "";		// Final response message

		const std::vector<std::string> Message = splitMessage(sMessage);

		const std::string sIdentifier = Message[0];
		const std::string sType = Message[1];
		const std::string sAction = Message[2];

		Logger::info(
			LogLevel::low,
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
			else if (sAction == "windowPos") {
				int x = std::stoi(Message[3]);
				int y = std::stoi(Message[4]);
				GLDRV->moveWindow(x, y);
			}
			else if (sAction == "windowSize") {
				int width = std::stoi(Message[3]);
				int height = std::stoi(Message[4]);
				GLDRV->resizeWindow(width, height);
			}
			else if (sAction == "ping") {
			}
			else if (sAction == "end") {
				DEMO->m_exitDemo = true;
			}
			else {
				sResult = "NOK";
				sInfo = "Unknown command (" + sAction + "): " + sMessage;
			}
		}
		else if (sType == "section") {
			// Sections processing
			if (sAction == "new") {
				if (DEMO->loadScriptFromNetwork(Message[3]) != 0) {
					sResult = "NOK";
					sInfo = "Section load failed";
				}
			}
			else if (sAction == "toggle") {
				DEMO->m_sectionManager.toggleSection(Message[3]);
			}
			else if (sAction == "delete") {
				DEMO->m_sectionManager.deleteSection(Message[3]);
			}
			else if (sAction == "update") {
				DEMO->m_sectionManager.updateSection(Message[3], Message[4]);
			}
			else if (sAction == "setStartTime") {
				DEMO->m_sectionManager.setSectionsStartTime(Message[3], Message[4]);
			}
			else if (sAction == "setEndTime") {
				DEMO->m_sectionManager.setSectionsEndTime(Message[3], Message[4]);
			}
			else if (sAction == "setLayer") {
				DEMO->m_sectionManager.setSectionLayer(Message[3], Message[4]);
			}
			else {
				sResult = "NOK";
				sInfo = "Unknown section command (" + sAction + "): " + sMessage;
			}
		}
		else {
			sResult = "NOK";
			sInfo = "Unknown network message type (" + sType + "): " + sMessage;
		}

		// Create response
		sResponse = sIdentifier + kDelimiterChar +
			sResult + kDelimiterChar +
			std::to_string(DEMO->m_fps) + kDelimiterChar +
			std::to_string(DEMO->m_status) + kDelimiterChar +
			std::to_string(DEMO->m_demoRunTime) + kDelimiterChar +
			sInfo;

		Logger::info(LogLevel::low, "Sending response: %s", sResponse.c_str());

		// return response
		return sResponse;
	}

	void NetDriver::sendMessage(std::string const& sMessage) const
	{
		if (!m_bConnectedToEditor_)
			return;

		dyad_write(m_pServConnect_, sMessage.c_str(), static_cast<int>(sMessage.size()));
	}


	// private static //////////////////////////////////////////////////////////////////////////////////

	void NetDriver::dyadOnData(dyad_Event* const pDyadEvent) {
		const std::string sResponse = NetDriver::GetInstance().processMessage(pDyadEvent->data);

		// Send the response and close the connection
		dyad_write(pDyadEvent->stream, sResponse.c_str(), static_cast<int>(sResponse.size()));
		dyad_end(pDyadEvent->stream);
	}

	void NetDriver::dyadOnAccept(dyad_Event* const pDyadEvent)
	{
		dyad_addListener(pDyadEvent->remote, DYAD_EVENT_DATA, dyadOnData, nullptr);
	}

	void NetDriver::dyadOnListen(dyad_Event* const pDyadEvent)
	{
		Logger::info(
			LogLevel::med,
			"Network listener started in port: %d",
			dyad_getPort(pDyadEvent->stream)
		);
	}

	void NetDriver::dyadOnError(dyad_Event* const pDyadEvent)
	{
		Logger::error("Network server error: %s", pDyadEvent->msg);
	}

	void NetDriver::dyadOnConnect(dyad_Event* const pDyadEvent)
	{
		NetDriver::GetInstance().m_bConnectedToEditor_ = true;

		Logger::info(
			LogLevel::med,
			"Network: Connected to editor through port: %d",
			dyad_getPort(pDyadEvent->stream)
		);
	}


	// private /////////////////////////////////////////////////////////////////////////////////////////

	std::vector<std::string> NetDriver::splitMessage(const std::string& message) const
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
}