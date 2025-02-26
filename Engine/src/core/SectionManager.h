// SectionManager.h
// Spontz Demogroup

#pragma once

#include "section.h"

namespace Phoenix {

	// Section info
	struct SectionInfo final {
		std::string m_id;
		SectionType m_type;
		std::function<Section* ()> m_fnCreateSection;
	};

	class SectionManager final {
		friend class DemoKernel;
		friend class SectionLayer;
		friend class ImGuiLayer;
		friend class sLoading;		// hack
		friend class SpoReader;		// hack

	public:
		~SectionManager();

	public:
		// Adds a Section of a given type
		int32_t addSection(SectionType type, std::string_view dataSource, bool enabled);
		void toggleSections(std::vector<std::string> const& ids);
		bool deleteSections(std::vector<std::string> const& ids);
		void updateSection(std::string_view id, std::string_view script);
		void setSectionsStartTime(std::vector<std::string> const& ids, float value);
		void setSectionsEndTime(std::vector<std::string> const& ids, float value);
		void setSectionLayer(std::string_view identifier, int32_t value);
		void clear(); // Clear all sections

	private:
		Section* getSection(std::string_view id) const;
		int32_t getSectionIndex(std::string_view id) const;
		std::vector<SectionInfo> getSectionTypes() const;

	private:
		// Sections list, script order
		std::vector<Section*> m_section;

		// Ready section list: Sections to be loaded (ascendent order by start time)
		std::vector<int32_t> m_loadSection;

		// Exec Render section list: Sections with Render capabilities to be executed this
		// frame (first element is the layer, and second the ID of the section)
		std::vector<std::pair<int32_t, int32_t>> m_execRenderSection;

		// Exec Sound section list: Sections with Sound capabilities to be executed this
		// frame (first element is the layer, and second the ID of the section)
		std::vector<std::pair<int32_t, int32_t>> m_execSoundSection; // Sound sections must be evaluated constantly, in order to be able to stop audio if not being executed

		// loading information
		int32_t m_LoadedSections	= 0;
		int32_t m_WarmedSections	= 0;
	};
}
