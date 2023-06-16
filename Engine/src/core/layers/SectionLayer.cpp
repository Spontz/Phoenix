// SectionLayer.cpp
// Spontz Demogroup

#include "Main.h"
#include "core/layers/SectionLayer.h"


namespace Phoenix {
	
	SectionLayer::SectionLayer(SectionManager* SectionManager) :
		Layer("SectionLayer"),
		m_SectionManager(SectionManager)
	{
	}
	void SectionLayer::OnAttach()
	{
		PX_PROFILE_FUNCTION();

	}

	void SectionLayer::OnDetach()
	{
		PX_PROFILE_FUNCTION();
	}

	void SectionLayer::OnEvent(Event& e)
	{
	}

	void SectionLayer::LoadSections()
	{
		Section* pSection = nullptr;
		Section* pLoadingSection = nullptr;

		int sec_id;

		// Set the demo state to loading
		DEMO->m_status = DemoStatus::LOADING;
		Logger::info(LogLevel::high, "Loading Start...");

		const auto startTime = DEMO->m_debug ? static_cast<float>(glfwGetTime()) : 0.0f;

		// Search for the loading section, if not found, we will create one
		for (size_t i = 0; i < m_SectionManager->m_section.size(); i++) {
			if (m_SectionManager->m_section[i]->type == SectionType::Loading)
				pLoadingSection = m_SectionManager->m_section[i];
		}

		if (pLoadingSection == nullptr) {
			Logger::info(LogLevel::med, "Loading section not found: using default loader");
			sec_id = m_SectionManager->addSection(SectionType::Loading, "Automatically created", true);
			if (sec_id < 0) {
				Logger::error("Critical Error, Loading section not found and could not be created!");
				return;
			}
			else {
				pLoadingSection = m_SectionManager->m_section[sec_id];
			}
		}

		// preload, load and init loading section
		pLoadingSection->load();
		pLoadingSection->init();
		pLoadingSection->loaded = true;
		pLoadingSection->inited = true;
		pLoadingSection->exec();

		{
			Logger::ScopedIndent _;
			Logger::info(LogLevel::med, "Loading section loaded, inited and executed for first time");

			// Clear the load and run section lists
			m_SectionManager->m_loadSection.clear();
			m_SectionManager->m_execSection.clear();

			// Populate Load Section: The sections that need to be loaded
			for (size_t i = 0; i < m_SectionManager->m_section.size(); i++) {
				pSection = m_SectionManager->m_section[i];
				// If we are in slave mode, we load all the sections but if not, we will load only the ones
				// that are inside the demo time
				if (DEMO->m_slaveMode == 1 || ((pSection->startTime < DEMO->m_demoEndTime || fabs(DEMO->m_demoEndTime) < FLT_EPSILON) && (pSection->endTime > startTime))) {
					// If the section is not the "loading", then we add id to the Ready Section lst
					if (pSection->type != SectionType::Loading) {
						m_SectionManager->m_loadSection.push_back(static_cast<int32_t>(i));
						// load section splines (to avoid code load in the sections)
						// loadSplines(ds); // TODO: Delete this once splines are working
					}
				}
			}

			Logger::info(LogLevel::low, "Ready Section queue complete: {} sections to be loaded",m_SectionManager->m_loadSection.size());

			// Start Loading the sections of the Ready List
			m_SectionManager->m_LoadedSections = 0;
			for (size_t i = 0; i < m_SectionManager->m_loadSection.size(); i++) {
				sec_id = m_SectionManager->m_loadSection[i];
				pSection = m_SectionManager->m_section[sec_id];
				if (pSection->load()) {
					pSection->loadDebugStatic(); // Load static debug info
					pSection->loaded = true;
				}
				// Incrmeent the loading sections even if it has not been sucesfully loaded, because
				//  it's just for the "loading" screen
				++m_SectionManager->m_LoadedSections;

				// Update loading
				pLoadingSection->exec();
				if (pSection->loaded)
					Logger::info(LogLevel::low,	"Section {} [id: {}, DataSource: {}] loaded OK!", sec_id, pSection->identifier, pSection->DataSource);
				else
					Logger::error("Section {} [id: {}, DataSource: {}] not loaded properly!", sec_id, pSection->identifier, pSection->DataSource);

				if (DEMO->m_exitDemo) {
					DEMO->Close();
					exit(EXIT_SUCCESS);
				}
			}
		}

		Logger::info(LogLevel::med,	"Loading complete, {} sections have been loaded.", m_SectionManager->m_LoadedSections);
	}

	void SectionLayer::ReInitSections()
	{
		Logger::ScopedIndent _;
		Logger::info(LogLevel::low, "Analysing sections that must be re-inited...");
		for (auto i = 0; i < m_SectionManager->m_execSection.size(); i++) {
			// The second value is the ID of the section
			const auto sec_id = m_SectionManager->m_execSection[i].second;
			const auto ds = m_SectionManager->m_section[sec_id];
			if ((ds->enabled) && (ds->loaded) && (ds->type != SectionType::Loading)) {
				ds->inited = FALSE; // Mark the section as not inited
				Logger::info(LogLevel::low, "Section {} [layer: {} id: {}] marked to be inited", sec_id, ds->layer, ds->identifier);
			}
		}
	}

	void SectionLayer::ProcessSections(float DemoRunTime)
	{
		// Check the sections that need to be executed
		Logger::ScopedIndent _;
		Logger::info(LogLevel::low, "Processing sections that must be executed...");
		m_SectionManager->m_execSection.clear();
		for (auto i = 0; i < m_SectionManager->m_section.size(); ++i) {
			const auto pSection = m_SectionManager->m_section[i];
			// If time is OK
			if (pSection->startTime <= DemoRunTime && pSection->endTime >= DemoRunTime) {
				// If its enabled, loaded and is not hte Loading section
				if (pSection->enabled && pSection->loaded && pSection->type != SectionType::Loading) {
					// Load the section: first the layer and then the ID
					m_SectionManager->m_execSection.push_back(std::make_pair(pSection->layer, i));
				}
			}
		}

		// Sort sections by Layer
		sort(m_SectionManager->m_execSection.begin(), m_SectionManager->m_execSection.end());

		Logger::info(LogLevel::low, "Section queue process complete: {} sections to be executed", m_SectionManager->m_execSection.size());
		// Run Init sections
		Logger::info(LogLevel::low, "Running Init Sections...");
		for (auto i = 0; i < m_SectionManager->m_execSection.size(); ++i) {
			// The second value is the ID of the section
			const auto sec_id = m_SectionManager->m_execSection[i].second;
			const auto ds = m_SectionManager->m_section[sec_id];
			if (ds->inited == FALSE) {
				ds->runTime = DemoRunTime - ds->startTime;
				ds->init();			// Init the Section
				ds->inited = TRUE;
				Logger::info(LogLevel::low, "Section {} [layer: {} id: {} type: {}] inited", sec_id, ds->layer, ds->identifier, ds->type_str);
			}
		}
	}

	void SectionLayer::ExecuteSections(float DemoRunTime)
	{
		// Run Exec sections
		Logger::ScopedIndent _;
		Logger::info(LogLevel::low, "Running Exec Sections...");
		for (auto i = 0; i < m_SectionManager->m_execSection.size(); ++i) {
			// The second value is the ID of the section
			const auto sec_id = m_SectionManager->m_execSection[i].second;
			const auto ds = m_SectionManager->m_section[sec_id];
			ds->runTime = DemoRunTime - ds->startTime;
			ds->exec();			// Exec the Section
			Logger::info(LogLevel::low, "Section {} [layer: {} id: {} type: {}] executed", sec_id, ds->layer, ds->identifier, ds->type_str);
		}
		Logger::info(LogLevel::med, "End queue processing!");
	}

	void SectionLayer::End()
	{
	}
}