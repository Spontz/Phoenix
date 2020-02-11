// sections.h
// Spontz Demogroup

#ifndef SECTIONS_H
#define SECTIONS_H

#include "core/section.h"

class sLoading : public Section { public:	sLoading();  bool load(); void init(); void exec(); void end(); string debug(); };
class sSound : public Section { public:	sSound();  bool load(); void init(); void exec(); void end(); string debug(); };
class sCamera : public Section { public:	sCamera();  bool load(); void init(); void exec(); void end(); string debug(); };
class sLight : public Section { public:	sLight();  bool load(); void init(); void exec(); void end(); string debug(); };
class sBackground : public Section { public:	sBackground();  bool load(); void init(); void exec(); void end(); string debug(); };
class sFboBind : public Section { public:	sFboBind();  bool load(); void init(); void exec(); void end(); string debug(); };
class sFboUnbind : public Section { public:	sFboUnbind();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawImage : public Section { public:	sDrawImage();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawFbo : public Section { public:	sDrawFbo();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawSkybox : public Section { public:	sDrawSkybox();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawVideo : public Section { public:	sDrawVideo();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawQuad : public Section { public:	sDrawQuad();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawScene : public Section { public:	sDrawScene();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawSceneMatrix : public Section { public:	sDrawSceneMatrix();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawParticles : public Section { public:	sDrawParticles();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawParticlesScene : public Section { public:	sDrawParticlesScene();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawEmitters : public Section { public:	sDrawEmitters();  bool load(); void init(); void exec(); void end(); string debug(); };
class sDrawEmitterScene : public Section { public:	sDrawEmitterScene();  bool load(); void init(); void exec(); void end(); string debug(); };
class sEfxBloom : public Section { public:	sEfxBloom();  bool load(); void init(); void exec(); void end(); string debug(); };
class sEfxBlur : public Section { public:	sEfxBlur();  bool load(); void init(); void exec(); void end(); string debug(); };
class sEfxFader : public Section { public:	sEfxFader();  bool load(); void init(); void exec(); void end(); string debug(); };
class sEfxMotionBlur : public Section { public:	sEfxMotionBlur();  bool load(); void init(); void exec(); void end(); string debug(); };
class sSetVariable : public Section { public:	sSetVariable();  bool load(); void init(); void exec(); void end(); string debug(); };
class sTest : public Section { public:	sTest();  bool load(); void init(); void exec(); void end(); string debug(); };

namespace sections {
	class SPH;
}

#endif
