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

	Efx_Accum,
	Efx_BlackWhite,
	Efx_BoxBlur,
	Efx_Fader,
	Efx_Fog,
	Efx_Glow,
	Efx_HighPassFilter,
	Efx_Inverse,
	Efx_MotionBlur,
	Efx_MotionBlurAccum,
	Efx_RadialBlur2,
	Efx_RadialGlow,
	Efx_RenderMesh2,

	Background,
	Video,
	Camera,
	Camera2,
	Camera3,
	Image,
	Image2,
	Image2D,
	ImageMatrix,
	ImagePart,
	Light,
	ObjectMorph,
	ObjectMorph2,
	ObjectShader,
	ObjectShader2,
	ObjectMatrix,
	ParticleMatrix2,
	ParticleMatrix3,
	ParticleTex2,
	Ray,
	RayMatrix,
	Sound,
	BeatDetect,
	SaveBuffer,
	RenderBuffer,
	Fbobind,
	Fbounbind,
	RenderFbo,
	GLSLShaderQuad,
	GLSLShaderBind,
	GLSLShaderUnbind,
	RenderShadowMapping
};



class SectionManager {
public:
	std::vector<Section*> section;	// Sections list, script order
	std::vector<int> loadSection;	// Ready section list: Sections to be loaded (ascendent order by start time)
	std::vector<pair<int, int>> execSection;	// Exec section list: Sections to be executed this frame (first element is the layer, and second the ID of the section)

	SectionManager();
	void init();
	int addSection(string key, string DataSource, int enabled);		// Adds a Section of a given type
private:
	SectionType getSectionType(string key);

	
};

#endif