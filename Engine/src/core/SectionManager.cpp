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
	int32_t SectionManager::addSection(
		const std::string& key,
		const std::string& DataSource,
		bool enabled
	)
	{
		Section* mySection = NULL;
		int32_t sec_id = -1;

		switch (getSectionType(key)) {
		case SectionType::Loading:
			mySection = instance_loading();
			break;
		case SectionType::Sound:
			mySection = instance_sound();
			break;
		case SectionType::SetVariable:
			mySection = instance_setVariable();
			break;
		case SectionType::CameraFPS:
			mySection = instance_cameraFPS();
			break;
		case SectionType::CameraTarget:
			mySection = instance_cameraTarget();
			break;
		case SectionType::LightSec:
			mySection = instance_light();
			break;
		case SectionType::FboBind:
			mySection = instance_fboBind();
			break;
		case SectionType::FboUnbind:
			mySection = instance_fboUnbind();
			break;
		case SectionType::DrawImage:
			mySection = instance_drawImage();
			break;
		case SectionType::DrawSkybox:
			mySection = instance_drawSkybox();
			break;
		case SectionType::DrawVideo:
			mySection = instance_drawVideo();
			break;
		case SectionType::DrawFbo:
			mySection = instance_drawFbo();
			break;
		case SectionType::DrawQuad:
			mySection = instance_drawQuad();
			break;
		case SectionType::DrawScene:
			mySection = instance_drawScene();
			break;
		case SectionType::DrawSceneMatrix:
			mySection = instance_drawSceneMatrix();
			break;
		case SectionType::DrawSceneMatrixInstanced:
			mySection = instance_drawSceneMatrixInstanced();
			break;
		case SectionType::DrawParticles:
			mySection = instance_drawParticles();
			break;
		case SectionType::DrawParticlesImage:
			mySection = instance_drawParticlesImage();
			break;
		case SectionType::DrawParticlesScene:
			mySection = instance_drawParticlesScene();
			break;
		case SectionType::DrawEmitters:
			mySection = instance_drawEmitters();
			break;
		case SectionType::DrawEmitterScene:
			mySection = instance_drawEmitterScene();
			break;
		case SectionType::EfxAccum:
			mySection = instance_efxAccum();
			break;
		break;	case SectionType::EfxBloom:
			mySection = instance_efxBloom();
			break;
		case SectionType::EfxBlur:
			mySection = instance_efxBlur();
			break;
		case SectionType::EfxFader:
			mySection = instance_efxFader();
			break;
		case SectionType::EfxMotionBlur:
			mySection = instance_efxMotionBlur();
			break;
		case SectionType::Test:
			mySection = instance_test();
			break;
		case SectionType::NOT_FOUND:
			break;
		}
		if (mySection != nullptr) {
			mySection->loaded = FALSE;	// By default, the section is not loaded
			mySection->enabled = enabled;
			mySection->DataSource = DataSource;
			mySection->type_str = key;
			m_section.push_back(mySection);
			sec_id = (int32_t)m_section.size() - 1;
		}
		return sec_id;
	}

	Section* SectionManager::getSection(const std::string& id) {
		int32_t sec_size = (int32_t)this->m_section.size();
		Section* ds;
		for (int32_t i = 0; i < sec_size; i++) {
			ds = this->m_section[i];
			if (ds->identifier == id) {
				return ds;
			}
		}
		return NULL;
	}

	int32_t SectionManager::getSectionPosition(const std::string& id) {
		int32_t sec_size = (int32_t)this->m_section.size();
		Section* ds;
		for (int32_t i = 0; i < sec_size; i++) {
			ds = this->m_section[i];
			if (ds->identifier == id) {
				return i;
			}
		}
		return NULL;
	}

	void SectionManager::toggleSection(const std::string& identifier)
	{
		std::vector<std::string> ids = splitIdentifiers(identifier);

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

	void SectionManager::deleteSection(const std::string& identifier)
	{
		std::vector<std::string> ids = splitIdentifiers(identifier);

		Section* ds;
		int32_t			ds_number;
		int32_t id_size = (int32_t)ids.size();

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			ds_number = getSectionPosition(ids[i]);
			if (ds) {
				ds->destroy();
				this->m_section.erase(this->m_section.begin() + ds_number);
				//Logger::sendEditor("Section %d [layer: %d id: %s type: %s] deleted", i, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
			}
			else {
				Logger::error("Section NOT deleted: %s", ids[i].c_str());
			}
		}
	}

	// Delete the old section and create a new one with the new parameters
	void SectionManager::updateSection(const std::string& identifier, const std::string& sScript)
	{
		this->deleteSection(identifier);
		DEMO->loadScriptFromNetwork(sScript);
	}

	std::vector<std::string> SectionManager::splitIdentifiers(const std::string& identifiers)
	{
		std::stringstream ss(identifiers);
		std::vector<std::string> result;

		while (ss.good())
		{
			std::string substr;
			getline(ss, substr, ',');
			result.push_back(substr);
		}
		return result;
	}

	void SectionManager::setSectionsStartTime(
		const std::string& amount,
		const std::string& identifiers
	)
	{
		std::vector<std::string> ids = splitIdentifiers(identifiers);
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

	void SectionManager::setSectionsEndTime(const std::string& amount, const std::string& identifiers)
	{
		std::vector<std::string> ids = splitIdentifiers(identifiers);
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

	void SectionManager::setSectionLayer(const std::string& layer, const std::string& identifier)
	{
		Section* ds;
		int32_t new_layer = atoi(layer.c_str());

		ds = getSection(identifier);
		if (ds) {
			//Logger::sendEditor("Section [%s] changed Layer from %d to %d", ds->identifier.c_str(), ds->layer, new_layer);
			ds->layer = new_layer;
		}
		else {
			Logger::error("Section NOT modified (setSectionLayer): %s", identifier.c_str());
		}
	}

	void SectionManager::clear()
	{
		Logger::info(LogLevel::med, "SectionManager::clear()");
		Logger::info(LogLevel::low, "{");
		for (auto const& pSection : m_section) {
			Logger::info(LogLevel::low, "Deleting %s...", pSection->identifier.c_str());
			delete pSection;
		}
		Logger::info(LogLevel::low, "}");

		m_section.clear();
		m_loadSection.clear();
		m_execSection.clear();
	}

	SectionType SectionManager::getSectionType(const std::string& key)
	{
		for (const auto& i : sectionID)
			if (key ==  i.scriptName)
				return i.type;

		return SectionType::NOT_FOUND;
	}
}