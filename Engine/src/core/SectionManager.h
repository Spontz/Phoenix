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
		int32_t addSection(const std::string& key, const std::string& DataSource, bool enabled);
		void toggleSection(const std::string& identifier);
		void deleteSection(const std::string& identifier);
		void updateSection(const std::string& identifier, const std::string& sScript);
		void setSectionsStartTime(const std::string& amount, const std::string& identifiers);
		void setSectionsEndTime(const std::string& amount, const std::string& identifiers);
		void setSectionLayer(const std::string& layer, const std::string& identifier);
		void clear(); // Clear all sections

	private:
		SectionType getSectionType(const std::string& key);
		std::vector<std::string> splitIdentifiers(const std::string& identifiers);
		Section* getSection(const std::string& id);
		int32_t getSectionPosition(const std::string& id);

	private:
		std::vector<Section*> m_section;	// Sections list, script order

		// Ready section list: Sections to be loaded (ascendent order by start time)
		std::vector<int32_t> m_loadSection;

		// Exec section list: Sections to be executed this frame (first element is the layer, and second
		// the ID of the section)
		std::vector<std::pair<int32_t, int32_t>> m_execSection;
	};
}
