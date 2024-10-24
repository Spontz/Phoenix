// NetDriver.cpp
// Spontz Demogroup

// includes ////////////////////////////////////////////////////////////////////////////////////////

#include "core/drivers/NetDriver.h"
#include "core/events/SectionEventManager.h"
#include "core/demokernel.h"

namespace Phoenix {
	// globals /////////////////////////////////////////////////////////////////////////////////////////
	auto const kDelimiterChar = '\x1f';
	auto const kDelimiterString = "\x1f";

	NetDriver* kpNetDriver=nullptr;

	// public static ///////////////////////////////////////////////////////////////////////////////////
	NetDriver& NetDriver::getInstance() {
		if (!kpNetDriver)
			kpNetDriver = new NetDriver();
		return *kpNetDriver;
	}

	void NetDriver::release() {
		delete kpNetDriver;
		kpNetDriver = nullptr;
	}


	// constructors/destructor /////////////////////////////////////////////////////////////////////////
	NetDriver::NetDriver()
		:
		m_iPortReceive(29000),
		m_iPortSend(29001),
		m_bInitialized(false),
		m_bConnectedToEditor(false),
		m_pServConnect(nullptr) {
	}

	NetDriver::~NetDriver() {
		dyad_shutdown();
	}


	// public

	void NetDriver::init() {
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

		m_bInitialized = true;
	}

	void NetDriver::connectToEditor() {
		// Listener for sending messages to the editor
		Logger::info(
			LogLevel::med,
			"Network: outgoing messages will be done through port: {}",
			m_iPortSend
		);

		m_pServConnect = dyad_newStream();
		dyad_addListener(m_pServConnect, DYAD_EVENT_CONNECT, dyadOnConnect, nullptr);
		dyad_connect(m_pServConnect, "127.0.0.1", m_iPortSend);
	}

	void NetDriver::update() const {
		dyad_update();
	}

	const char* NetDriver::getVersion() const {
		return dyad_getVersion();
	}

	std::string NetDriver::processMessage(const std::string& sMessage) const {
		// Outcoming information
		std::string sResult = "OK";		// Result of the operation
		std::string sInfo = "";			// Information message
		std::string sResponse = "";		// Final response message

		std::string sIdentifier;
		std::string sType;
		std::string sAction;
		
		const std::vector<std::string> Message = splitMessage(sMessage);

		if (Message.size() < 2) {
			sIdentifier = "00";
			sType = "unknown";
			sAction = "unknown";
		}
		else {
			sIdentifier = Message[0];
			sType = Message[1];
			sAction = Message[2];
		}		

		Logger::info(LogLevel::low, "Message received: [identifier: {}] [type: {}] [action: {}]", sIdentifier, sType, sAction);

		if (sType == "command") {
			NetDriver::DemoAction demoAction;
			demoAction = getDemoAction(sAction.c_str());
			switch (demoAction) {
			case DemoAction::PING:
				sResult = "pong";
				break;
			case DemoAction::PAUSE:
				DEMO->pauseDemo();
				break;
			case DemoAction::PLAY:
				DEMO->playDemo();
				break;
			case DemoAction::RESTART:
				DEMO->restartDemo();
				break;
			case DemoAction::SET_STARTTIME: {
				float time = std::stof(Message[3]);
				DEMO->setStartTime(time);
				break;
			}
			case DemoAction::SET_CURRENTTIME: {
				float time = std::stof(Message[3]);
				DEMO->setCurrentTime(time);
				break;
			}
			case DemoAction::SET_ENDTIME: {
				float time = std::stof(Message[3]);
				DEMO->setEndTime(time);
				break;
			}
			case DemoAction::SET_WINDOWPOS: {
				int32_t x = std::stoi(Message[3]);
				int32_t y = std::stoi(Message[4]);
				WindowMoveEvent WinMoveEvent(x, y);
				DEMO->OnEvent(WinMoveEvent);
				break;
			}
			case DemoAction::SET_WINDOWSIZE: {
				uint32_t width = std::stoul(Message[3]);
				uint32_t height = std::stoul(Message[4]);
				WindowResizeEvent WinResizeEvent(width, height);
				DEMO->OnEvent(WinResizeEvent);
				break;
			}
			case DemoAction::END: {
				WindowCloseEvent WinClosed;
				DEMO->OnEvent(WinClosed);
				break;
			}
			default:
				sResult = "NOK";
				sInfo = "Unknown command (" + sAction + "): " + sMessage;
				break;
			}
		} else if (sType == "section") {
			std::string msg1 = "";
			std::string msg2 = "";
			
			if (Message.size() > 3)
				msg1 = Message[3];
			if (Message.size() > 4)
				msg2 = Message[4];

			if (!DEMO->m_sectionEventManager.addSectionEvent(sAction, msg1, msg2)) {
				sResult = "NOK";
				sInfo = "Unknown section command (" + sAction + "): " + sMessage;
			}
		} else {
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

		Logger::info(LogLevel::low, "Sending response: {}", sResponse);

		// return response
		return sResponse;
	}

	void NetDriver::sendMessage(std::string const& sMessage) const {
		if (!m_bConnectedToEditor)
			return;

		dyad_write(m_pServConnect, sMessage.c_str(), static_cast<int>(sMessage.size()));
	}

	NetDriver::DemoAction NetDriver::getDemoAction(std::string_view Action) const
	{
		if (m_NetDemoAction.find(Action) != m_NetDemoAction.end())
			return m_NetDemoAction[Action];
		else
			return DemoAction::UNKNOWN;
	}

	// private static //////////////////////////////////////////////////////////////////////////////////
	void NetDriver::dyadOnData(dyad_Event* const pDyadEvent) {
		const std::string sResponse = NetDriver::getInstance().processMessage(pDyadEvent->data);

		// Send the response and close the connection
		dyad_write(pDyadEvent->stream, sResponse.c_str(), static_cast<int>(sResponse.size()));
		dyad_end(pDyadEvent->stream);
	}

	void NetDriver::dyadOnAccept(dyad_Event* const pDyadEvent) {
		dyad_addListener(pDyadEvent->remote, DYAD_EVENT_DATA, dyadOnData, nullptr);
	}

	void NetDriver::dyadOnListen(dyad_Event* const pDyadEvent) {
		Logger::info(
			LogLevel::med,
			"Network listener started in port: {}",
			dyad_getPort(pDyadEvent->stream)
		);
	}

	void NetDriver::dyadOnError(dyad_Event* const pDyadEvent) {
		Logger::error("Network server error: {}", pDyadEvent->msg);
	}

	void NetDriver::dyadOnConnect(dyad_Event* const pDyadEvent) {
		NetDriver::getInstance().m_bConnectedToEditor = true;

		Logger::info(
			LogLevel::med,
			"Network: Connected to editor through port: {}",
			dyad_getPort(pDyadEvent->stream)
		);
	}

	// private /////////////////////////////////////////////////////////////////////////////////////////
	std::vector<std::string> NetDriver::splitMessage(const std::string& message) const {
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
