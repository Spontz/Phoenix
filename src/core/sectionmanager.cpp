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
	{"cameraAbs",			SectionType::CameraAbsSec},
	{"light",				SectionType::LightSec},
	{"drawScene",			SectionType::DrawScene},
	{"drawSceneMatrix",		SectionType::DrawSceneMatrix},
	{"drawImage",			SectionType::DrawImage},
	{"drawSkybox",			SectionType::DrawSkybox},
	{"drawVideo",			SectionType::DrawVideo},
	{"drawQuad",			SectionType::DrawQuad},
	{"drawFbo",				SectionType::DrawFbo},
	{"drawParticles",		SectionType::DrawParticles},
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
	case SectionType::SetVariable:
		mySection = new sSetVariable();
		break;
	case SectionType::CameraSec:
		mySection = new sCamera();
		break;
	case SectionType::CameraAbsSec:
		mySection = new sCameraAbs();
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
	case SectionType::DrawImage:
		mySection = new sDrawImage();
		break;
	case SectionType::DrawSkybox:
		mySection = new sDrawSkybox();
		break;
	case SectionType::DrawVideo:
		mySection = new sDrawVideo();
		break;
	case SectionType::DrawFbo:
		mySection = new sDrawFbo();
		break;
	case SectionType::DrawQuad:
		mySection = new sDrawQuad();
		break;
	case SectionType::DrawScene:
		mySection = new sDrawScene();
		break;
	case SectionType::DrawSceneMatrix:
		mySection = new sDrawSceneMatrix();
		break;
	case SectionType::DrawParticles:
		mySection = new sDrawParticles();
		break;
	case SectionType::DrawParticlesScene:
		mySection = new sDrawParticlesScene();
		break;
	case SectionType::DrawEmitters:
		mySection = new sDrawEmitters();
		break;
	case SectionType::DrawEmitterScene:
		mySection = new sDrawEmitterScene();
		break;
	case SectionType::EfxAccum:
		mySection = new sEfxAccum();
		break;
	break;	case SectionType::EfxBloom:
		mySection = new sEfxBloom();
		break;
	case SectionType::EfxBlur:
		mySection = new sEfxBlur();
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

Section* SectionManager::getSection(string id) {
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

int SectionManager::getSectionPosition(string id) {
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

void SectionManager::toggleSection(string identifier)
{
	vector<string> ids = splitIdentifiers(identifier);

	Section *ds;
	int id_size = (int)ids.size();
	
	for (int i = 0; i < id_size; i++) {
		ds = getSection(ids[i]);
		if (ds) {
			ds->enabled = !ds->enabled;
			//LOG->SendEditor("Section toggled: %s", ids[i].c_str());
		}
		else {
			LOG->Error("Section NOT toggled: %s", ids[i].c_str());
		}
	}
}

void SectionManager::deleteSection(string identifier)
{
	vector<string> ids = splitIdentifiers(identifier);

	Section*	ds;
	int			ds_number;
	int id_size = (int)ids.size();

	for (int i = 0; i < id_size; i++) {
		ds = getSection(ids[i]);
		ds_number = getSectionPosition(ids[i]);
		if (ds) {
			ds->end();
			this->section.erase(this->section.begin() + ds_number);
			//LOG->SendEditor("Section %d [layer: %d id: %s type: %s] deleted", i, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
		}
		else {
			LOG->Error("Section NOT deleted: %s", ids[i].c_str());
		}
	}
}

// Delete the old section and create a new one with the new parameters
void SectionManager::updateSection(string identifier, string sScript)
{
	this->deleteSection(identifier);
	DEMO->load_scriptFromNetwork(sScript);
}

std::vector<string> SectionManager::splitIdentifiers(string identifiers) {
	std::stringstream ss(identifiers);
	vector<string> result;

	while (ss.good())
	{
		string substr;
		getline(ss, substr, ',');
		result.push_back(substr);
	}
	return result;
}

void SectionManager::setSectionsStartTime(string amount, string identifiers)
{
	vector<string> ids = splitIdentifiers(identifiers);
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
			//LOG->SendEditor("Section [%s] changed StartTime: %.3f", ds->identifier.c_str(), ds->startTime);
		}
		else {
			LOG->Error("Section NOT modified (StartTime): %s", ids[i].c_str());
		}
	}
}

void SectionManager::setSectionsEndTime(string amount, string identifiers)
{
	vector<string> ids = splitIdentifiers(identifiers);
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
			//LOG->SendEditor("Section [%s] changed EndTime: %.3f", ds->identifier.c_str(), ds->endTime);
		}
		else {
			LOG->Error("Section NOT modified (EndTime): %s", ids[i].c_str());
		}
	}
}

void SectionManager::setSectionLayer(string layer, string identifier)
{
	Section *ds;
	int new_layer = atoi(layer.c_str());

	ds = getSection(identifier);
	if (ds) {
		//LOG->SendEditor("Section [%s] changed Layer from %d to %d", ds->identifier.c_str(), ds->layer, new_layer);
		ds->layer = new_layer;
	}
	else {
		LOG->Error("Section NOT modified (setSectionLayer): %s", identifier.c_str());
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