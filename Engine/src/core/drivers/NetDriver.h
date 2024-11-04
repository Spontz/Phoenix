// NetDriver.h
// Spontz Demogroup

#pragma once

#include "core/drivers/net/dyad.h"
#include "core/events/SectionEventManager.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace Phoenix {

	class NetDriver final {

		enum class DemoAction {
			PING = 0,
			PAUSE,
			PLAY,
			RESTART,
			SET_STARTTIME,
			SET_CURRENTTIME,
			SET_ENDTIME,
			SET_WINDOWPOS,
			SET_WINDOWSIZE,
			END,
			UNKNOWN
		};

		mutable std::unordered_map<std::string, DemoAction> m_NetDemoAction = {
			{"ping",		DemoAction::PING},
			{"pause",		DemoAction::PAUSE},
			{"play",		DemoAction::PLAY},
			{"restart",		DemoAction::RESTART},
			{"startTime",	DemoAction::SET_STARTTIME},
			{"currentTime",	DemoAction::SET_CURRENTTIME},
			{"endTime",		DemoAction::SET_ENDTIME},
			{"windowPos",	DemoAction::SET_WINDOWPOS},
			{"windowSize",	DemoAction::SET_WINDOWSIZE},
			{"end",			DemoAction::END}
		};

	public:
		static NetDriver& getInstance();
		static void release();

	private:
		NetDriver();
		~NetDriver();

	public:
		void init();
		void connectToEditor();
		void update() const;
		const char* getVersion() const;
		std::string processMessage(const std::string& sMessage) const;
		void sendMessage(std::string const& sMessage) const;

	private:
		NetDriver::DemoAction getDemoAction(std::string_view Action) const;

		// Dyad Callbacks
		static void dyadOnData(dyad_Event* const pDyadEvent);
		static void dyadOnAccept(dyad_Event* const pDyadEvent);
		static void dyadOnListen(dyad_Event* const pDyadEvent);
		static void dyadOnError(dyad_Event* const pDyadEvent);
		static void dyadOnConnect(dyad_Event* const pDyadEvent);

	private:
		std::vector<std::string> splitMessage(const std::string& message) const;

	public:
		int32_t m_iPortReceive;
		int32_t m_iPortSend;

	private:
		bool m_bInitialized;
		bool m_bConnectedToEditor;
		dyad_Stream* m_pServConnect;
	};
}
