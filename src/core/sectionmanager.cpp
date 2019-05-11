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
	{"video",				SectionType::Video},
	{"camera",				SectionType::sCamera},
	{"camera2",				SectionType::sCamera2},
	{"camera3",				SectionType::sCamera3},
	{"image",				SectionType::Image},
	{"image2",				SectionType::Image2},
	{"image2D",				SectionType::Image2D},
	{"imageMatrix",			SectionType::ImageMatrix},
	{"imagePart",			SectionType::ImagePart},
	{"light",				SectionType::Light},
	{"objectMorph",			SectionType::ObjectMorph},
	{"objectMorph2",		SectionType::ObjectMorph2},
	{"objectShader",		SectionType::ObjectShader},
	{"objectShader2",		SectionType::ObjectShader2},
	{"objectMatrix",		SectionType::ObjectMatrix },
	{"particleMatrix2",		SectionType::ParticleMatrix2},
	{"particleMatrix3",		SectionType::ParticleMatrix3},
	{"particleTex2",		SectionType::ParticleTex2},
	{"ray",					SectionType::Ray},
	{"rayMatrix",			SectionType::RayMatrix},
	{"sound",				SectionType::Sound},
	{"beatDetect",			SectionType::BeatDetect},
	{"fbobind",				SectionType::FboBind},
	{"fbounbind",			SectionType::FboUnbind},
	{"renderfbo",			SectionType::RenderFbo},
	{"glslshaderquad",		SectionType::GLSLShaderQuad},
	{"glslshaderbind",		SectionType::GLSLShaderBind},
	{"glslshaderunbind",	SectionType::GLSLShaderUnbind},
	{"renderShadowMapping",	SectionType::RenderShadowMapping}
};

#define SECTIONS_NUMBER ((sizeof(sectionID) / sizeof(tSectionID)))


// Init vars
SectionManager::SectionManager() {
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
	case SectionType::GLSLShaderBind:
		mySection = new sGLSLShaderBind();
		break;
	case SectionType::GLSLShaderUnbind:
		mySection = new sGLSLShaderUnbind();
		break;
	case SectionType::ObjectShader:
		mySection = new sObjectShader();
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

SectionType SectionManager::getSectionType(string key) {

	int i;

	// get section index
	for (i = 0; i < SECTIONS_NUMBER; i++) {
		if (_strcmpi(key.c_str(), sectionID[i].scriptName) == 0) {
			return sectionID[i].type;
		}
	}

	return SectionType::NOT_FOUND;
}