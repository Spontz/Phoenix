// sections.h
// Spontz Demogroup

#pragma once

#include "core/section.h"

namespace Phoenix {
	Section* instance_loading();
	Section* instance_sound();
	Section* instance_cameraFPS();
	Section* instance_cameraTarget();
	Section* instance_light();
	Section* instance_fboBind();
	Section* instance_fboUnbind();
	Section* instance_drawImage();
	Section* instance_drawFbo();
	Section* instance_drawSkybox();
	Section* instance_drawVideo();
	Section* instance_drawQuad();
	Section* instance_drawScene();
	Section* instance_drawSceneMatrix();
	Section* instance_drawSceneMatrixInstanced();
	Section* instance_drawParticles();
	Section* instance_drawParticlesImage();
	Section* instance_drawParticlesScene();
	Section* instance_drawEmitterScene();
	Section* instance_drawEmitterSceneEx();
	Section* instance_drawEmitterSpline();
	Section* instance_efxAccum();
	Section* instance_efxBloom();
	Section* instance_efxBlur();
	Section* instance_efxFader();
	Section* instance_efxMotionBlur();
	Section* instance_setVariable();
	Section* instance_test();

	namespace sections {
		class SPH;
	}
}