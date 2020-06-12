// sectionmanager.h
// Spontz Demogroup

#ifndef SECTIONMANAGER_H
#define SECTIONMANAGER_H
#include <string>
#include <vector>
#include "section.h"
#include "sections/sections.h"

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
	EfxAccum,
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
	std::vector<std::pair<int, int>> execSection;	// Exec section list: Sections to be executed this frame (first element is the layer, and second the ID of the section)

	SectionManager();
	virtual ~SectionManager();

	int addSection(std::string key, std::string DataSource, int enabled);		// Adds a Section of a given type
	void	toggleSection(std::string identifier);
	void	deleteSection(std::string identifier);
	void	updateSection(std::string identifier, std::string sScript);
	void	setSectionsStartTime(std::string amount, std::string identifiers);
	void	setSectionsEndTime(std::string amount, std::string identifiers);
	void	setSectionLayer(std::string layer, std::string identifier);

private:
	SectionType getSectionType(std::string key);
	std::vector<std::string> splitIdentifiers(std::string identifiers);
	Section* getSection(std::string id);
	int getSectionPosition(std::string id);
};

#endif