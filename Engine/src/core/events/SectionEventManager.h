// SectionEventQueue.h
// Spontz Demogroup

#pragma once

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

#include "core/events/SectionEvent.h"

namespace Phoenix {

	class SectionEventManager final {

		friend class ImGuiLayer;

		mutable std::unordered_map<std::string, SectionEventType> m_SectionEvents = {
			{"new",			SectionEventType::SEC_NEW},
			{"toggle",		SectionEventType::SEC_TOGGLE},
			{"delete",		SectionEventType::SEC_DELETE},
			{"update",		SectionEventType::SEC_UPDATE},
			{"setStartTime",SectionEventType::SEC_SET_STARTTIME},
			{"setEndTime",	SectionEventType::SEC_SET_ENDTIME},
			{"setLayer",	SectionEventType::SEC_SET_LAYER},
		};

	public:
		SectionEventManager();
		~SectionEventManager();

		SectionEventType getSectionEventType(std::string_view type) const;
		bool addSectionEvent(std::string_view EventType, std::string_view Message1, std::string_view Message2);
		void executeEvents();
		void clear();
		int32_t getNumEventsExecutionQueue();
		int32_t getNumEventsAdditionQueue();

	private:
		void clearQueue(uint32_t queue);

	public:
		std::vector<SectionEvent*>	m_SectionEventQueue[2];		// double buffer queue: One being executed, and the other being added
		uint8_t						m_executionQueue;			// The queue of events under execution
		uint8_t						m_additionQueue;			// The queue used for adding new events

	};
}