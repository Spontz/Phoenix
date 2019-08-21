// sections.h
// Spontz Demogroup

#ifndef SECTIONS_H
#define SECTIONS_H

#include "core/section.h"

class sLoading : public Section { public:	sLoading();  bool load(); void init(); void exec(); void end(); };
class sSound : public Section { public:	sSound();  bool load(); void init(); void exec(); void end(); };
class sCamera : public Section { public:	sCamera();  bool load(); void init(); void exec(); void end(); };
class sLight : public Section { public:	sLight();  bool load(); void init(); void exec(); void end(); };
class sBackground : public Section { public:	sBackground();  bool load(); void init(); void exec(); void end(); };
class sFboBind : public Section { public:	sFboBind();  bool load(); void init(); void exec(); void end(); };
class sFboUnbind : public Section { public:	sFboUnbind();  bool load(); void init(); void exec(); void end(); };
class sDrawImage : public Section { public:	sDrawImage();  bool load(); void init(); void exec(); void end(); };
class sDrawFbo : public Section { public:	sDrawFbo();  bool load(); void init(); void exec(); void end(); };
class sDrawSkybox : public Section { public:	sDrawSkybox();  bool load(); void init(); void exec(); void end(); };
class sDrawVideo : public Section { public:	sDrawVideo();  bool load(); void init(); void exec(); void end(); };
class sDrawQuad : public Section { public:	sDrawQuad();  bool load(); void init(); void exec(); void end(); };
class sObjectShader : public Section { public:	sObjectShader();  bool load(); void init(); void exec(); void end(); };
class sObjectAnimShader : public Section { public:	sObjectAnimShader();  bool load(); void init(); void exec(); void end(); };
class sParticleMatrix : public Section { public:	sParticleMatrix();  bool load(); void init(); void exec(); void end(); };
class sEfxBloom : public Section { public:	sEfxBloom();  bool load(); void init(); void exec(); void end(); };
class sEfxFader : public Section { public:	sEfxFader();  bool load(); void init(); void exec(); void end(); };
class sEfxMotionBlur : public Section { public:	sEfxMotionBlur();  bool load(); void init(); void exec(); void end(); };
class sTest : public Section { public:	sTest();  bool load(); void init(); void exec(); void end(); };

namespace sections {
	class SPH;
}

#endif
