// SectionManager.h
// Spontz Demogroup

#pragma once

#include "section.h"

namespace Phoenix {

	class SectionManager final {
		friend class demokernel;
		friend class imGuiDriver; // hack
		friend class sLoading; // hack
		friend class SpoReader; // hack

	public:
		~SectionManager();

	public:
		// Adds a Section of a given type
		int32_t addSection(SectionType type, std::string_view dataSource, bool enabled);
		void toggleSections(std::vector<std::string> const& ids);
		void deleteSections(std::vector<std::string> const& ids);
		void updateSection(std::string_view id, std::string_view script);
		void setSectionsStartTime(std::vector<std::string> const& ids, float value);
		void setSectionsEndTime(std::vector<std::string> const& ids, float value);
		void setSectionLayer(std::string_view identifier, int32_t value);
		void clear(); // Clear all sections

	private:
		Section* getSection(std::string_view id) const;
		int32_t getSectionPosition(std::string_view id) const;

	private:
		std::vector<Section*> m_section; // Sections list, script order

		// Ready section list: Sections to be loaded (ascendent order by start time)
		std::vector<int32_t> m_loadSection;

		// Exec section list: Sections to be executed this frame (first element is the layer, and second
		// the ID of the section)
		std::vector<std::pair<int32_t, int32_t>> m_execSection;
	};
}
