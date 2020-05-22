// sectionmanager.h
// Spontz Demogroup

#ifndef SECTIONMANAGER_H
#define SECTIONMANAGER_H
#include <string>
#include <vector>
#include "section.h"
#include "sections/sections.h"

using namespace std;

enum SectionType {
	NOT_FOUND,	// No section found

	Loading,

	Background,
	VideoSec,
	CameraSec,
	CameraAbsSec,
	LightSec, 
	ObjectShader,
	ObjectAnimShader,
	DrawImage,
	DrawSkybox,
	DrawVideo,
	DrawQuad,
	DrawFbo,
	DrawScene,
	DrawSceneMatrix,
	DrawParticles,
	DrawParticlesScene,
	DrawEmitters,
	DrawEmitterScene,
	SetVariable,
	Sound,
	FboBind,
	FboUnbind,
	EfxBloom,
	EfxBlur,
	EfxFader,
	EfxMotionBlur,
	Test
};



class SectionManager {
public:
	std::vector<Section*> section;	// Sections list, script order
	std::vector<int> loadSection;	// Ready section list: Sections to be loaded (ascendent order by start time)
	std::vector<pair<int, int>> execSection;	// Exec section list: Sections to be executed this frame (first element is the layer, and second the ID of the section)

	SectionManager();
	virtual ~SectionManager();

	int addSection(string key, string DataSource, int enabled);		// Adds a Section of a given type
	void	toggleSection(string identifier);
	void	deleteSection(string identifier);
	void	updateSection(string identifier, string sScript);
	void	setSectionsStartTime(string amount, string identifiers);
	void	setSectionsEndTime(string amount, string identifiers);
	void	setSectionLayer(string layer, string identifier);

private:
	SectionType getSectionType(string key);
	std::vector<string> splitIdentifiers(string identifiers);
	Section* getSection(string id);
	int getSectionPosition(string id);
};

#endif