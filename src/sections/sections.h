// sections.h
// Spontz Demogroup

#ifndef SECTIONS_H
#define SECTIONS_H
#include <string>
#include <vector>
#include "core/section.h"
using namespace std;


class sLoading : public Section { public:	sLoading();  bool load(); void init(); void exec(); void end(); };
class sSound : public Section { public:	sSound();  bool load(); void init(); void exec(); void end(); };
class sCamera : public Section { public:	sCamera();  bool load(); void init(); void exec(); void end(); };
class sLight : public Section { public:	sLight();  bool load(); void init(); void exec(); void end(); };
class sBackground : public Section { public:	sBackground();  bool load(); void init(); void exec(); void end(); };
class sFboBind : public Section { public:	sFboBind();  bool load(); void init(); void exec(); void end(); };
class sFboUnbind : public Section { public:	sFboUnbind();  bool load(); void init(); void exec(); void end(); };
class sRenderFbo : public Section { public:	sRenderFbo();  bool load(); void init(); void exec(); void end(); };
class sDrawSkybox : public Section { public:	sDrawSkybox();  bool load(); void init(); void exec(); void end(); };
class sDrawVideo : public Section { public:	sDrawVideo();  bool load(); void init(); void exec(); void end(); };
class sRenderQuad : public Section { public:	sRenderQuad();  bool load(); void init(); void exec(); void end(); };
class sObjectShader : public Section { public:	sObjectShader();  bool load(); void init(); void exec(); void end(); };
class sObjectAnimShader : public Section { public:	sObjectAnimShader();  bool load(); void init(); void exec(); void end(); };
class sParticleMatrix : public Section { public:	sParticleMatrix();  bool load(); void init(); void exec(); void end(); };
class sTest : public Section { public:	sTest();  bool load(); void init(); void exec(); void end(); };

#endif