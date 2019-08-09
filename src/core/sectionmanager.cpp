// sectionmanager.cpp
// Spontz Demogroup

#include "main.h"
#include "sectionmanager.h"
#include "sections/sections.h"


typedef struct {
	char *scriptName;
	SectionType type;
} tSectionID;

// sections functions references
tSectionID sectionID[] = {
	{"loading", SectionType::Loading },

	// built-in sections
	{"background",			SectionType::Background},
	{"camera",				SectionType::CameraSec},
	{"light",				SectionType::LightSec},
	{"objectShader",		SectionType::ObjectShader},
	{"objectAnimShader",	SectionType::ObjectAnimShader},
	{"drawSkybox",			SectionType::DrawSkybox},
	{"drawVideo",			SectionType::DrawVideo},
	{"particleMatrix",		SectionType::ParticleMatrix},
	{"sound",				SectionType::Sound},
	{"fboBind",				SectionType::FboBind},
	{"fboUnbind",			SectionType::FboUnbind},
	{"renderFbo",			SectionType::RenderFbo},
	{"renderQuad",			SectionType::RenderQuad},
	{"efxBloom",			SectionType::EfxBloom},
	{"efxFader",			SectionType::EfxFader},
	{"efxMotionblur",		SectionType::EfxMotionBlur},
	{"test",				SectionType::Test},
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
int SectionManager::addSection(string key, string DataSource, int enabled) {
	
	Section *mySection = NULL;
	int sec_id = -1;

	switch (getSectionType(key)) {
	case SectionType::Loading:
		mySection = new sLoading();
		break;
	case SectionType::Sound:
		mySection = new sSound();
		break;
	case SectionType::CameraSec:
		mySection = new sCamera();
		break;
	case SectionType::LightSec:
		mySection = new sLight();
		break;
	case SectionType::Background:
		mySection = new sBackground();
		break;
	case SectionType::FboBind:
		mySection = new sFboBind();
		break;
	case SectionType::FboUnbind:
		mySection = new sFboUnbind();
		break;
	case SectionType::RenderFbo:
		mySection = new sRenderFbo();
		break;
	case SectionType::RenderQuad:
		mySection = new sRenderQuad();
		break;
	case SectionType::ObjectShader:
		mySection = new sObjectShader();
		break;
	case SectionType::ObjectAnimShader:
		mySection = new sObjectAnimShader();
		break;
	case SectionType::DrawSkybox:
		mySection = new sDrawSkybox();
		break;
	case SectionType::DrawVideo:
		mySection = new sDrawVideo();
		break;
	case SectionType::ParticleMatrix:
		mySection = new sParticleMatrix();
		break;
	case SectionType::EfxBloom:
		mySection = new sEfxBloom();
		break;
	case SectionType::EfxFader:
		mySection = new sEfxFader();
		break;
	case SectionType::EfxMotionBlur:
		mySection = new sEfxMotionBlur();
		break;
	case SectionType::Test:
		mySection = new sTest();
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

void SectionManager::toggleSection(string identifier)
{
	Section *ds;
	int size = (int)this->section.size();
	for (int i = 0; i < size; i++) {
		ds = this->section[i];
		if (ds->identifier == identifier) {
			ds->enabled = !ds->enabled;
			return;
		}
	}
}

void SectionManager::deleteSection(string identifier)
{
	Section *ds;
	int size = (int)this->section.size();
	for (int i = 0; i < size; i++) {
		ds = this->section[i];
		if (ds->identifier == identifier) {
			LOG->Info(LOG_LOW, "  Section %d [layer: %d id: %s type: %s] deleted", i, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
			ds->end();
			this->section.erase(this->section.begin() + i);
			return;
		}
	}
}

// Delete the old section and create a new one with the new parameters
void SectionManager::updateSection(string identifier, string sScript)
{
	this->deleteSection(identifier);
	DEMO->load_scriptFromNetwork(sScript);
}

void SectionManager::setSectionsStartTime(string amount, string identifiers)
{
	std::vector<string> identifier;
	std::stringstream ss(identifiers);
	string ident;

	// TODO: Seems that this is not needed, sections are always sent one by one... Ask Ivan if the editor is sending the secitons one bye one or splitted by comas

	// Split the identifiers, separated by coma, and put it into "identifier" vector
	while (ss >> ident)
	{
		identifier.push_back(ident);

		if (ss.peek() == ',')
			ss.ignore();
	}

	// Get the new startTime
	Section *ds;
	int sec_size = (int)this->section.size();
	int ident_size = (int)identifier.size();
	float startTime = (float)atof(amount.c_str());


	for (int i = 0; i < sec_size; i++) {
		ds = this->section[i];
		for (int j = 0; j < ident_size; j++) {
			if (ds->identifier == identifier[j]) {
				ds->startTime = startTime;
				ds->duration = ds->endTime - ds->startTime;
				// Reload the splines. This way they are recalculated
				for (int k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->duration = ds->duration;
					ds->spline[k]->load();
				}
				LOG->Info(LOG_LOW, "Section [%s] changed StartTime: %.3f", ds->identifier.c_str(), ds->startTime);

			}
		}
	}
}

void SectionManager::setSectionsEndTime(string amount, string identifiers)
{
	std::vector<string> identifier;
	std::stringstream ss(identifiers);
	string ident;

	// Split the identifiers, separated by coma, and put it into "identifier" vector
	while (ss >> ident)
	{
		identifier.push_back(ident);

		if (ss.peek() == ',')
			ss.ignore();
	}

	// Get the new startTime
	Section *ds;
	int sec_size = (int)this->section.size();
	int ident_size = (int)identifier.size();
	float endTime = (float)atof(amount.c_str());

	for (int i = 0; i < sec_size; i++) {
		ds = this->section[i];
		for (int j = 0; j < ident_size; j++) {
			if (ds->identifier == identifier[j]) {
				ds->endTime = endTime;
				ds->duration = ds->endTime - ds->startTime;
				// Reload the splines. This way they are recalculated
				for (int k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->duration = ds->duration;
					ds->spline[k]->load();
				}
				LOG->Info(LOG_LOW, "Section [%s] changed EndTime: %.3f", ds->identifier.c_str(), ds->endTime);
			}
		}
	}
}

void SectionManager::setSectionLayer(string layer, string identifier)
{
	Section *ds;
	int new_layer = atoi(layer.c_str());
	int size = (int)this->section.size();
	for (int i = 0; i < size; i++) {
		ds = this->section[i];
		if (ds->identifier == identifier) {
			LOG->Info(LOG_LOW, "Section [%s] changed Layer from %d to %d", ds->identifier.c_str(), ds->layer, new_layer);
			ds->layer = new_layer;
			return;
		}
	}
}

SectionType SectionManager::getSectionType(string key)
{
	int i;

	for (i = 0; i < SECTIONS_NUMBER; i++) {
		if (_strcmpi(key.c_str(), sectionID[i].scriptName) == 0) {
			return sectionID[i].type;
		}
	}

	return SectionType::NOT_FOUND;
}