// SectionManager.cpp
// Spontz Demogroup

#include "main.h"
#include "SectionManager.h"
#include "sections/sections.h"


typedef struct {
	char *scriptName;
	SectionType type;
} tSectionID;

// sections functions references
tSectionID sectionID[] = {
	{"loading", SectionType::Loading },

	// built-in sections
	{"camera",				SectionType::CameraSec},
	{"cameraAbs",			SectionType::CameraAbsSec},
	{"light",				SectionType::LightSec},
	{"drawScene",			SectionType::DrawScene},
	{"drawSceneMatrix",		SectionType::DrawSceneMatrix},
	{"DrawSceneMatrixInstanced",		SectionType::DrawSceneMatrixInstanced},
	{"drawImage",			SectionType::DrawImage},
	{"drawSkybox",			SectionType::DrawSkybox},
	{"drawVideo",			SectionType::DrawVideo},
	{"drawQuad",			SectionType::DrawQuad},
	{"drawFbo",				SectionType::DrawFbo},
	{"drawParticles",		SectionType::DrawParticles},
	{"drawParticlesImage",	SectionType::DrawParticlesImage},
	{"drawParticlesScene",	SectionType::DrawParticlesScene},
	{"drawEmitters",		SectionType::DrawEmitters},
	{"drawEmitterScene",	SectionType::DrawEmitterScene},
	{"sound",				SectionType::Sound},
	{"setVariable",			SectionType::SetVariable},
	{"fboBind",				SectionType::FboBind},
	{"fboUnbind",			SectionType::FboUnbind},
	{"efxAccum",			SectionType::EfxAccum},
	{"efxBloom",			SectionType::EfxBloom},
	{"efxBlur",				SectionType::EfxBlur},
	{"efxFader",			SectionType::EfxFader},
	{"efxMotionblur",		SectionType::EfxMotionBlur},
	{"test",				SectionType::Test}
};

#define SECTIONS_NUMBER ((sizeof(sectionID) / sizeof(tSectionID)))


// Init vars
SectionManager::SectionManager() {
	this->section.clear();
	this->loadSection.clear();
	this->execSection.clear();
}

SectionManager::~SectionManager()
{
	this->section.clear();
	this->loadSection.clear();
	this->execSection.clear();
}


// Adds a Section into the queue
// Returns the section ID or -1 if the section could not be added
int SectionManager::addSection(const std::string& key, const std::string& DataSource, int enabled) {
	
	Section *mySection = NULL;
	int sec_id = -1;

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
	case SectionType::CameraSec:
		mySection = instance_camera();
		break;
	case SectionType::CameraAbsSec:
		mySection = instance_cameraAbsSection();
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
	if (mySection != NULL) {
		mySection->loaded = FALSE;	// By default, the section is not loaded
		mySection->enabled = enabled;
		mySection->DataSource = DataSource;
		mySection->type_str = key;
		section.push_back(dynamic_cast<Section*>(mySection));
		sec_id = (int)section.size() - 1;
	}
	return sec_id;
}

Section* SectionManager::getSection(const std::string& id) {
	int sec_size = (int)this->section.size();
	Section* ds;
	for (int i = 0; i < sec_size; i++) {
		ds = this->section[i];
		if (ds->identifier == id) {
			return ds;
		}
	}
	return NULL;
}

int SectionManager::getSectionPosition(const std::string& id) {
	int sec_size = (int)this->section.size();
	Section* ds;
	for (int i = 0; i < sec_size; i++) {
		ds = this->section[i];
		if (ds->identifier == id) {
			return i;
		}
	}
	return NULL;
}

void SectionManager::toggleSection(const std::string& identifier)
{
	std::vector<std::string> ids = splitIdentifiers(identifier);

	Section *ds;
	int id_size = (int)ids.size();
	
	for (int i = 0; i < id_size; i++) {
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

	Section*	ds;
	int			ds_number;
	int id_size = (int)ids.size();

	for (int i = 0; i < id_size; i++) {
		ds = getSection(ids[i]);
		ds_number = getSectionPosition(ids[i]);
		if (ds) {
			ds->end();
			this->section.erase(this->section.begin() + ds_number);
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
	DEMO->load_scriptFromNetwork(sScript);
}

std::vector<std::string> SectionManager::splitIdentifiers(const std::string& identifiers) {
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

void SectionManager::setSectionsStartTime(const std::string& amount, const std::string& identifiers)
{
	std::vector<std::string> ids = splitIdentifiers(identifiers);
	float startTime = (float)atof(amount.c_str());

	Section* ds;
	int id_size = (int)ids.size();

	for (int i = 0; i < id_size; i++) {
		ds = getSection(ids[i]);
		if (ds) {
			ds->startTime = startTime;
			ds->duration = ds->endTime - ds->startTime;
			// Reload the splines. This way they are recalculated
			for (int k = 0; k < ds->spline.size(); k++) {
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
	int id_size = (int)ids.size();

	for (int i = 0; i < id_size; i++) {
		ds = getSection(ids[i]);
		if (ds) {
			ds->endTime = endTime;
			ds->duration = ds->endTime - ds->startTime;
			// Reload the splines. This way they are recalculated
			for (int k = 0; k < ds->spline.size(); k++) {
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
	Section *ds;
	int new_layer = atoi(layer.c_str());

	ds = getSection(identifier);
	if (ds) {
		//Logger::sendEditor("Section [%s] changed Layer from %d to %d", ds->identifier.c_str(), ds->layer, new_layer);
		ds->layer = new_layer;
	}
	else {
		Logger::error("Section NOT modified (setSectionLayer): %s", identifier.c_str());
	}
}

SectionType SectionManager::getSectionType(const std::string& key)
{
	int i;

	for (i = 0; i < SECTIONS_NUMBER; i++) {
		if (_strcmpi(key.c_str(), sectionID[i].scriptName) == 0) {
			return sectionID[i].type;
		}
	}

	return SectionType::NOT_FOUND;
}