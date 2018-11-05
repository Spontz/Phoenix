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
	NOT_FOUND,	// No seciton found

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
	std::vector<Section*> section;

	SectionManager();
	void init();
	int addSection(string key, string DataSource, int enabled);
private:
	SectionType getSectionType(string key);

	
};

#endif