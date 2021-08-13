// SectionManager.cpp
// Spontz Demogroup

#include "main.h"
#include "SectionManager.h"
#include "sections/sections.h"

namespace Phoenix {

	struct tSectionID {
		std::string scriptName;
		SectionType type;
	};

	// sections functions references
	const tSectionID sectionID[] = {
		{"loading",						SectionType::Loading},

		// built-in sections
		{"cameraFPS",					SectionType::CameraFPS},
		{"cameraTarget",				SectionType::CameraTarget},
		{"light",						SectionType::LightSec},
		{"drawScene",					SectionType::DrawScene},
		{"drawSceneMatrix",				SectionType::DrawSceneMatrix},
		{"drawSceneMatrixInstanced",	SectionType::DrawSceneMatrixInstanced},
		{"drawImage",					SectionType::DrawImage},
		{"drawSkybox",					SectionType::DrawSkybox},
		{"drawVideo",					SectionType::DrawVideo},
		{"drawQuad",					SectionType::DrawQuad},
		{"drawFbo",						SectionType::DrawFbo},
		{"drawParticles",				SectionType::DrawParticles},
		{"drawParticlesImage",			SectionType::DrawParticlesImage},
		{"drawParticlesScene",			SectionType::DrawParticlesScene},
		{"drawEmitters",				SectionType::DrawEmitters},
		{"drawEmitterScene",			SectionType::DrawEmitterScene},
		{"sound",						SectionType::Sound},
		{"setVariable",					SectionType::SetVariable},
		{"fboBind",						SectionType::FboBind},
		{"fboUnbind",					SectionType::FboUnbind},
		{"efxAccum",					SectionType::EfxAccum},
		{"efxBloom",					SectionType::EfxBloom},
		{"efxBlur",						SectionType::EfxBlur},
		{"efxFader",					SectionType::EfxFader},
		{"efxMotionBlur",				SectionType::EfxMotionBlur},
		{"test",						SectionType::Test}
	};

	constexpr auto SECTIONS_NUMBER = sizeof(sectionID) / sizeof(tSectionID);

	SectionManager::~SectionManager()
	{
		Logger::info(LogLevel::med, "Destructing SectionManager...");
		clear();
	}

	// Adds a Section into the queue
	// Returns the section ID or -1 if the section could not be added
	int32_t SectionManager::addSection(SectionType type, std::string_view dataSource, bool enabled)
	{
		Section* pNewSection;

		switch (type) {
		case SectionType::Loading:
			pNewSection = instance_loading();
			break;
		case SectionType::Sound:
			pNewSection = instance_sound();
			break;
		case SectionType::SetVariable:
			pNewSection = instance_setVariable();
			break;
		case SectionType::CameraFPS:
			pNewSection = instance_cameraFPS();
			break;
		case SectionType::CameraTarget:
			pNewSection = instance_cameraTarget();
			break;
		case SectionType::LightSec:
			pNewSection = instance_light();
			break;
		case SectionType::FboBind:
			pNewSection = instance_fboBind();
			break;
		case SectionType::FboUnbind:
			pNewSection = instance_fboUnbind();
			break;
		case SectionType::DrawImage:
			pNewSection = instance_drawImage();
			break;
		case SectionType::DrawSkybox:
			pNewSection = instance_drawSkybox();
			break;
		case SectionType::DrawVideo:
			pNewSection = instance_drawVideo();
			break;
		case SectionType::DrawFbo:
			pNewSection = instance_drawFbo();
			break;
		case SectionType::DrawQuad:
			pNewSection = instance_drawQuad();
			break;
		case SectionType::DrawScene:
			pNewSection = instance_drawScene();
			break;
		case SectionType::DrawSceneMatrix:
			pNewSection = instance_drawSceneMatrix();
			break;
		case SectionType::DrawSceneMatrixInstanced:
			pNewSection = instance_drawSceneMatrixInstanced();
			break;
		case SectionType::DrawParticles:
			pNewSection = instance_drawParticles();
			break;
		case SectionType::DrawParticlesImage:
			pNewSection = instance_drawParticlesImage();
			break;
		case SectionType::DrawParticlesScene:
			pNewSection = instance_drawParticlesScene();
			break;
		case SectionType::DrawEmitters:
			pNewSection = instance_drawEmitters();
			break;
		case SectionType::DrawEmitterScene:
			pNewSection = instance_drawEmitterScene();
			break;
		case SectionType::EfxAccum:
			pNewSection = instance_efxAccum();
			break;
		case SectionType::EfxBloom:
			pNewSection = instance_efxBloom();
			break;
		case SectionType::EfxBlur:
			pNewSection = instance_efxBlur();
			break;
		case SectionType::EfxFader:
			pNewSection = instance_efxFader();
			break;
		case SectionType::EfxMotionBlur:
			pNewSection = instance_efxMotionBlur();
			break;
		case SectionType::Test:
			pNewSection = instance_test();
			break;

		default:
			return -1;
		}

		pNewSection->loaded = FALSE;	// By default, the section is not loaded
		pNewSection->enabled = enabled;
		pNewSection->DataSource = dataSource;
		pNewSection->type = type;
		m_section.push_back(pNewSection);
		return static_cast<int32_t>(m_section.size()) - 1;
	}

	Section* SectionManager::getSection(std::string_view id) const {
		int32_t sec_size = static_cast<int32_t>(m_section.size());
		Section* ds;
		for (int32_t i = 0; i < sec_size; i++) {
			ds = this->m_section[i];
			if (ds->identifier == id) {
				return ds;
			}
		}
		return NULL;
	}

	int32_t SectionManager::getSectionPosition(std::string_view id) const {
		int32_t sec_size = static_cast<int32_t>(m_section.size());
		Section* ds;
		for (int32_t i = 0; i < sec_size; i++) {
			ds = this->m_section[i];
			if (ds->identifier == id) {
				return i;
			}
		}
		return NULL;
	}

	void SectionManager::toggleSections(std::vector<std::string> const& ids)
	{
		Section* ds;
		int32_t id_size = (int32_t)ids.size();

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			if (ds) {
				ds->enabled = !ds->enabled;
				//Logger::sendEditor("Section toggled: %s", ids[i].c_str());
			}
			else {
				Logger::error("Section NOT toggled: %s", ids[i].c_str());
			}
		}
	}

	void SectionManager::deleteSections(std::vector<std::string> const& ids)
	{
		Section* ds;
		int32_t			ds_number;
		int32_t id_size = (int32_t)ids.size();

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			ds_number = getSectionPosition(ids[i]);
			if (ds) {
				this->m_section.erase(this->m_section.begin() + ds_number);
				//Logger::sendEditor("Section %d [layer: %d id: %s type: %s] deleted", i, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
			}
			else {
				Logger::error("Section NOT deleted: %s", ids[i].c_str());
			}
		}
	}

	// Delete the old section and create a new one with the new parameters
	void SectionManager::updateSection(std::string_view identifier, std::string_view sScript)
	{
		deleteSections({ identifier.data() });
		DEMO->loadScriptFromNetwork(sScript);
	}



	void SectionManager::setSectionsStartTime(
		const std::string& amount,
		std::vector<std::string> const& ids
	)
	{
		float startTime = (float)atof(amount.c_str());

		Section* ds;
		int32_t id_size = (int32_t)ids.size();

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			if (ds) {
				ds->startTime = startTime;
				ds->duration = ds->endTime - ds->startTime;
				// Reload the splines. This way they are recalculated
				for (int32_t k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->duration = ds->duration;
					ds->spline[k]->load();
				}
				//Logger::sendEditor("Section [%s] changed StartTime: %.3f", ds->identifier.c_str(), ds->startTime);
			}
			else {
				Logger::error("Section NOT modified (StartTime): %s", ids[i].c_str());
			}
		}
	}

	void SectionManager::setSectionsEndTime(const std::string& amount, std::vector<std::string> const& ids)
	{
		float endTime = (float)atof(amount.c_str());

		Section* ds;
		int32_t id_size = (int32_t)ids.size();

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			if (ds) {
				ds->endTime = endTime;
				ds->duration = ds->endTime - ds->startTime;
				// Reload the splines. This way they are recalculated
				for (int32_t k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->duration = ds->duration;
					ds->spline[k]->load();
				}
				//Logger::sendEditor("Section [%s] changed EndTime: %.3f", ds->identifier.c_str(), ds->endTime);
			}
			else {
				Logger::error("Section NOT modified (EndTime): %s", ids[i].c_str());
			}
		}
	}

	void SectionManager::setSectionLayer(std::string_view layer, std::string_view identifier)
	{
		Section* ds;
		int32_t new_layer = atoi(layer.data());

		ds = getSection(identifier);
		if (ds) {
			//Logger::sendEditor("Section [%s] changed Layer from %d to %d", ds->identifier.c_str(), ds->layer, new_layer);
			ds->layer = new_layer;
		}
		else {
			Logger::error("Section NOT modified (setSectionLayer): %s", identifier.data());
		}
	}

	void SectionManager::clear()
	{
		for (auto const& pSection : m_section) {
			Logger::info(LogLevel::low, "Deleting section: %s...", pSection->identifier.c_str());
			delete pSection;
		}

		m_section.clear();
		m_loadSection.clear();
		m_execSection.clear();
	}

	SectionType getSectionType(std::string_view key)
	{
		for (const auto& i : sectionID)
			if (key == i.scriptName)
				return i.type;

		return SectionType::NOT_FOUND;
	}

	std::ostream& operator<<(std::ostream& os, SectionType type)
	{
		for (const auto& i : sectionID)
			if (type == i.type) {
				os << i.scriptName;
				return os;
			}
		os << "SectionType::NOT_FOUND";
		return os;
	}


	std::string str(SectionType value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

}
