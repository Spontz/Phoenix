// SectionManager.h
// Spontz Demogroup

#pragma once

#include "section.h"

namespace Phoenix {

	// Parameter type that we can found in a section configuration
	enum class SectionVariableType {
		STRING = 0,
		PARAMETER,
		SPLINE,
		EXPRESSION_VARIABLE,
		EXPRESSION_CONSTANT
	};

	// Section strings, parameters and expression variables
	struct SectionVariable final {
		SectionType			SectionType;
		std::string			Name;
		SectionVariableType	Type;
		std::string			Description;
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
		std::vector<SectionVariable> getSectionVariablesInfo(SectionType type) const;

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

		// Table with all the variable information of each section
		static const SectionVariable kSectionVariableInfo[];
	};
}
