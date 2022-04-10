// SectionManager.cpp
// Spontz Demogroup

#include "main.h"
#include "SectionManager.h"
#include "sections/sections.h"

namespace Phoenix {

	struct SectionInfo final {
		std::string m_id;
		SectionType m_type;
		std::function<Section*()> m_fnCreateSection;
	};

	// sections functions references
	const SectionInfo kSectionInfo[] = {
		{"loading",						SectionType::Loading, instance_loading},

		// built-in sections
		{"cameraFPS",					SectionType::CameraFPS, instance_cameraFPS},
		{"cameraTarget",				SectionType::CameraTarget, instance_cameraTarget},
		{"light",						SectionType::LightSec, instance_light},
		{"drawScene",					SectionType::DrawScene, instance_drawScene},
		{"drawSceneMatrix",				SectionType::DrawSceneMatrix, instance_drawSceneMatrix},
		{"drawSceneMatrixInstanced",	SectionType::DrawSceneMatrixInstanced, instance_drawSceneMatrixInstanced},
		{"drawImage",					SectionType::DrawImage, instance_drawImage},
		{"drawSkybox",					SectionType::DrawSkybox, instance_drawSkybox},
		{"drawVideo",					SectionType::DrawVideo, instance_drawVideo},
		{"drawQuad",					SectionType::DrawQuad, instance_drawQuad},
		{"drawFbo",						SectionType::DrawFbo, instance_drawFbo},
		{"drawParticles",				SectionType::DrawParticles, instance_drawParticles},
		{"drawParticlesImage",			SectionType::DrawParticlesImage, instance_drawParticlesImage},
		{"drawParticlesScene",			SectionType::DrawParticlesScene, instance_drawParticlesScene},
		{"drawEmitterScene",			SectionType::DrawEmitterScene, instance_drawEmitterScene},
		{"drawEmitterSceneEx",			SectionType::DrawEmitterSceneEx, instance_drawEmitterSceneEx},
		{"sound",						SectionType::Sound, instance_sound},
		{"setVariable",					SectionType::SetVariable, instance_setVariable},
		{"fboBind",						SectionType::FboBind, instance_fboBind},
		{"fboUnbind",					SectionType::FboUnbind, instance_fboUnbind},
		{"efxAccum",					SectionType::EfxAccum, instance_efxAccum},
		{"efxBloom",					SectionType::EfxBloom, instance_efxBloom},
		{"efxBlur",						SectionType::EfxBlur, instance_efxBlur},
		{"efxFader",					SectionType::EfxFader, instance_efxFader},
		{"efxMotionBlur",				SectionType::EfxMotionBlur, instance_efxMotionBlur},
		{"test",						SectionType::Test, instance_test}
	};

	const SectionInfo* getSectionInfo(SectionType type) {
		static std::map<SectionType, const SectionInfo*> m;
		if (m.empty())
			for (auto const& i : kSectionInfo)
				m[i.m_type] = &i;

		const auto it = m.find(type);
		if (it == m.end())
			return nullptr;

		return it->second;
	}

	const SectionInfo* getSectionInfo(std::string_view id) {
		static std::map<std::string_view, const SectionInfo*> m;
		if (m.empty())
			for (auto const& i : kSectionInfo)
				m[i.m_id] = &i;

		const auto it = m.find(id);
		if (it == m.end())
			return nullptr;

		return it->second;
	}

	SectionManager::~SectionManager()
	{
		Logger::info(LogLevel::med, "Destructing SectionManager...");
		clear();
	}

	// Adds a Section into the queue
	// Returns the section ID or -1 if the section could not be added
	int32_t SectionManager::addSection(SectionType type, std::string_view dataSource, bool enabled)
	{
		const auto pSectionInfo = getSectionInfo(type);
		if (!pSectionInfo)
			return -1;

		const auto pNewSection = pSectionInfo->m_fnCreateSection();
		pNewSection->loaded = FALSE; // By default, the section is not loaded
		pNewSection->enabled = enabled;
		pNewSection->DataSource = dataSource;
		pNewSection->type = type;
		pNewSection->type_str = getSectionInfo(type)->m_id;
		m_section.push_back(pNewSection);

		return static_cast<int32_t>(m_section.size()) - 1;
	}

	Section* SectionManager::getSection(std::string_view id) const
	{
		// hack: slow
		for (auto pSection : m_section)
			if (pSection->identifier == id)
				return pSection;

		return nullptr;
	}

	int32_t SectionManager::getSectionIndex(std::string_view id) const
	{
		// hack: slow
		for (size_t i = 0; i < m_section.size(); i++)
			if (m_section[i]->identifier == id)
				return static_cast<int32_t>(i);

		return -1;
	}

	void SectionManager::toggleSections(std::vector<std::string> const& ids)
	{
		for (auto const& id : ids) {
			const auto pSection = getSection(id);
			if (pSection) {
				pSection->enabled = !pSection->enabled;
				//Logger::sendEditor("Section toggled: %s", ids[i].c_str());
			}
			else {
				Logger::error("Section NOT toggled: {}", id);
			}
		}
	}

	bool SectionManager::deleteSections(std::vector<std::string> const& ids)
	{
		bool failed = false;

		for (auto const& sectionId : ids)
			if (getSection(sectionId)) {
				int i = getSectionIndex(sectionId);
				if (i>=0)
				{
					delete m_section[i];
					m_section.erase(m_section.begin() + i);
					//Logger::sendEditor("Section %d [layer: %d id: %s type: %s] deleted", i, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
				}
				Logger::error("Section NOT deleted: {}", sectionId);
				failed = true;
			}
			else {
				Logger::error("Section NOT deleted: {}", sectionId);
				failed = true;
			}

		return failed;
	}

	// Delete the old section and create a new one with the new parameters
	void SectionManager::updateSection(std::string_view identifier, std::string_view sScript)
	{
		deleteSections({ identifier.data() });
		DEMO->loadScriptFromNetwork(sScript);
	}



	void SectionManager::setSectionsStartTime(std::vector<std::string> const& ids, float startTime)
	{
		Section* ds;
		int32_t id_size = (int32_t)ids.size();

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			if (ds) {
				ds->startTime = startTime;
				ds->duration = ds->endTime - ds->startTime;
				// Reload the splines. This way they are recalculated
				for (size_t k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->duration = ds->duration;
					ds->spline[k]->load();
				}
				//Logger::sendEditor("Section [%s] changed StartTime: %.3f", ds->identifier.c_str(), ds->startTime);
			}
			else {
				Logger::error("Section NOT modified (StartTime): {}", ids[i]);
			}
		}
	}

	void SectionManager::setSectionsEndTime(std::vector<std::string> const& ids, float endTime)
	{
		Section* ds;
		int32_t id_size = (int32_t)ids.size();

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			if (ds) {
				ds->endTime = endTime;
				ds->duration = ds->endTime - ds->startTime;
				// Reload the splines. This way they are recalculated
				for (size_t k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->duration = ds->duration;
					ds->spline[k]->load();
				}
				//Logger::sendEditor("Section [%s] changed EndTime: %.3f", ds->identifier.c_str(), ds->endTime);
			}
			else {
				Logger::error("Section NOT modified (EndTime): {}", ids[i]);
			}
		}
	}

	void SectionManager::setSectionLayer(std::string_view identifier, int32_t new_layer)
	{
		Section* ds;

		ds = getSection(identifier);
		if (ds) {
			//Logger::sendEditor("Section [%s] changed Layer from %d to %d", ds->identifier.c_str(), ds->layer, new_layer);
			ds->layer = new_layer;
		}
		else {
			Logger::error("Section NOT modified (setSectionLayer): {}", identifier);
		}
	}

	void SectionManager::clear()
	{
		for (auto const& pSection : m_section) {
			Logger::info(LogLevel::low, "Deleting section: {}...", pSection->identifier);
			delete pSection;
		}

		m_section.clear();
		m_loadSection.clear();
		m_execSection.clear();
	}

	SectionType getSectionType(std::string_view key)
	{
		const auto pSectionInfo = getSectionInfo(key);
		return pSectionInfo ? pSectionInfo->m_type : SectionType::NOT_FOUND;
	}

	std::ostream& operator<<(std::ostream& os, SectionType type)
	{
		const auto pSectionInfo = getSectionInfo(type);
		os << pSectionInfo ? pSectionInfo->m_id : "SectionType::NOT_FOUND";
		return os;
	}

	std::string str(SectionType value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

}
