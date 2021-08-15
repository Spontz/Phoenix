// NetDriver.h
// Spontz Demogroup

#pragma once

#include "core/drivers/net/dyad.h"

#include <string>
#include <vector>

namespace Phoenix {

	class NetDriver final {
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
