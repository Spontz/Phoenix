// SectionEvent.h
// Spontz Demogroup

#pragma once

#include <vector>
#include <string>

namespace Phoenix {

	enum class SectionEventType {
		SEC_NEW = 0,
		SEC_TOGGLE,
		SEC_DELETE,
		SEC_UPDATE,
		SEC_SET_STARTTIME,
		SEC_SET_ENDTIME,
		SEC_SET_LAYER,
		SEC_UNKNOWN
	};

	class SectionEvent {
		friend class DemoKernel;

	public:
		SectionEvent();
		virtual ~SectionEvent() = default;

		void	loadMessages(std::string_view Message1, std::string_view Message2);
		void	exec();		// Execute the event

	protected:
		std::vector<std::string> splitIdentifiers(std::string_view identifiers);

	public:
		SectionEventType	m_action;	// Action to be performed
		std::string			m_actionStr;// Action to be performed (in string format)
		bool				m_handled;	// Section event has been handled? if true, event should not be executed, and should be deleted from queue

		// The actions can include up to 2 strings
		std::string			m_Message1;
		std::string			m_Message2;
	
	protected:
		DemoKernel& m_demo;

	};
}