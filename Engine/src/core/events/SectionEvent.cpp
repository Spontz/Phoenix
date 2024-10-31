// SectionEventQueue.cpp
// Spontz Demogroup

#include "main.h"
#include "core/events/SectionEventManager.h"
#include "core/events/SectionEvent.h"
#include "core/events/SectionEventManager.h"


namespace Phoenix {

	SectionEvent::SectionEvent()
		:
		m_demo(*DEMO),
		m_handled(false),
		m_action(SectionEventType::SEC_UNKNOWN),
		m_actionStr("Unknown")
	{
	}

	std::vector<std::string> SectionEvent::splitIdentifiers(std::string_view identifiers)
	{
		std::stringstream ss(identifiers.data());
		std::vector<std::string> result;

		while (ss.good()) {
			std::string substr;
			getline(ss, substr, ',');
			result.emplace_back(substr);
		}
		return result;
	}

	void SectionEvent::loadMessages(std::string_view Message1, std::string_view Message2)
	{
		m_Message1 = Message1.data();
		m_Message2 = Message2.data();
	}

	void SectionEvent::exec()
	{
		switch (m_action) {
			
		case SectionEventType::SEC_NEW:
			m_demo.loadScriptFromNetwork(m_Message1);
			break;

		case SectionEventType::SEC_TOGGLE:
			m_demo.m_sectionManager.toggleSections(splitIdentifiers(m_Message1));
			break;

		case SectionEventType::SEC_DELETE:
			m_demo.m_sectionManager.deleteSections(splitIdentifiers(m_Message1));
			break;

		case SectionEventType::SEC_UPDATE:
			m_demo.m_sectionManager.updateSection(m_Message1, m_Message2);
			break;

		case SectionEventType::SEC_SET_STARTTIME:
			m_demo.m_sectionManager.setSectionsStartTime(
				splitIdentifiers(m_Message2),
				static_cast<float>(atof(m_Message1.data()))
			);
			break;
		
		case SectionEventType::SEC_SET_ENDTIME:
			m_demo.m_sectionManager.setSectionsEndTime(
				splitIdentifiers(m_Message2),
				static_cast<float>(atof(m_Message1.data()))
			);
			break;

		case SectionEventType::SEC_SET_LAYER:
			m_demo.m_sectionManager.setSectionLayer(m_Message2, atoi(m_Message1.data()));
			break;

		default:
			break;
		}
	}
}


