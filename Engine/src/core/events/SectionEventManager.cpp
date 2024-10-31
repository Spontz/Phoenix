// SectionEventQueue.cpp
// Spontz Demogroup

#include "core/events/SectionEventManager.h"
#include "core/events/SectionEvent.h"
#include "SectionEventManager.h"
#include "main.h"

namespace Phoenix {

	SectionEventManager::SectionEventManager()
		:
		m_additionQueue(0),
		m_executionQueue(1)
	{
	}

	SectionEventManager::~SectionEventManager()
	{
		m_SectionEventQueue[0].clear();
		m_SectionEventQueue[1].clear();
	}

	SectionEventType SectionEventManager::getSectionEventType(std::string_view type) const
	{
		if (m_SectionEvents.find(type) != m_SectionEvents.end())
			return m_SectionEvents[type];
		else
			return SectionEventType::SEC_UNKNOWN;
	}

	bool SectionEventManager::addSectionEvent(std::string_view EventType, std::string_view Message1, std::string_view Message2)
	{
		SectionEventType sectionEventType = getSectionEventType(EventType);
		if (sectionEventType == SectionEventType::SEC_UNKNOWN)
			return false;

		SectionEvent* pNewSectionEvent = new SectionEvent();
		pNewSectionEvent->loadMessages(Message1, Message2); // Load the message info
		pNewSectionEvent->m_action = sectionEventType;
		pNewSectionEvent->m_actionStr = EventType;

		// We add the event in the "add" Queue
		m_SectionEventQueue[m_additionQueue].emplace_back(pNewSectionEvent);

		// Add event to the log message
		std::stringstream ss;
		ss << "Type: " << EventType << std::endl;
		DEMO->ImGuiAddEventLogMessage(ss.str());

		return true;
	}
	void SectionEventManager::executeEvents()
	{
		if ((m_SectionEventQueue[m_executionQueue].empty()) && (m_SectionEventQueue[m_additionQueue].empty()))
			return;

		// We only execute the execution queue if we have something insisde
		if (!m_SectionEventQueue[m_executionQueue].empty()) {
			for (auto& pEvent : m_SectionEventQueue[m_executionQueue]) {
				pEvent->exec();
			}
			clearQueue(m_executionQueue);
		}
		
		// Swap buffers
		m_additionQueue = m_executionQueue;
		m_executionQueue = (m_executionQueue + 1) & 0x1;
	}

	void SectionEventManager::clearQueue(uint32_t queue)
	{
		for (auto const& pSectionEvent : m_SectionEventQueue[queue]) {
			delete pSectionEvent;
		}
		m_SectionEventQueue[queue].clear();

	}

	void SectionEventManager::clear()
	{
		for (uint32_t i = 0; i < 2; i++) {
			clearQueue(i);
		}
	}

	int32_t SectionEventManager::getNumEventsExecutionQueue()
	{
		return static_cast<int32_t>(m_SectionEventQueue[m_executionQueue].size());
	}

	int32_t SectionEventManager::getNumEventsAdditionQueue()
	{
		return static_cast<int32_t>(m_SectionEventQueue[m_additionQueue].size());
	}


}


