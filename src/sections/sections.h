// sections.h
// Spontz Demogroup

#ifndef SECTIONS_H
#define SECTIONS_H

#include "core/section.h"

Section* instance_loading();
Section* instance_sound();
Section* instance_camera();
Section* instance_cameraAbsSection();
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
Section* instance_drawEmitters();
Section* instance_drawEmitterScene();
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

#endif
