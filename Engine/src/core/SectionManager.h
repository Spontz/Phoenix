// SectionManager.h
// Spontz Demogroup

#pragma once

#include <string>
#include <vector>
#include "section.h"
#include "sections/sections.h"

namespace Phoenix {

	class SectionManager {
	public:
		std::vector<Section*> section;	// Sections list, script order
		std::vector<int> loadSection;	// Ready section list: Sections to be loaded (ascendent order by start time)
		std::vector<std::pair<int, int>> execSection;	// Exec section list: Sections to be executed this frame (first element is the layer, and second the ID of the section)

		SectionManager();
		virtual ~SectionManager();

		int addSection(const std::string& key, const std::string& DataSource, int enabled);		// Adds a Section of a given type
		void	toggleSection(const std::string& identifier);
		void	deleteSection(const std::string& identifier);
		void	updateSection(const std::string& identifier, const std::string& sScript);
		void	setSectionsStartTime(const std::string& amount, const std::string& identifiers);
		void	setSectionsEndTime(const std::string& amount, const std::string& identifiers);
		void	setSectionLayer(const std::string& layer, const std::string& identifier);

	private:
		SectionType getSectionType(const std::string& key);
		std::vector<std::string> splitIdentifiers(const std::string& identifiers);
		Section* getSection(const std::string& id);
		int getSectionPosition(const std::string& id);
	};
}