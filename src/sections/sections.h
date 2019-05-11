// sections.h
// Spontz Demogroup

#ifndef SECTIONS_H
#define SECTIONS_H
#include <string>
#include <vector>
#include "core/section.h"
using namespace std;


class sLoading : public Section { public:	sLoading();  void load(); void init(); void exec(); void end(); };
class sSound : public Section { public:	sSound();  void load(); void init(); void exec(); void end(); };
class sBackground : public Section { public:	sBackground();  void load(); void init(); void exec(); void end(); };
class sFboBind : public Section { public:	sFboBind();  void load(); void init(); void exec(); void end(); };
class sFboUnbind : public Section { public:	sFboUnbind();  void load(); void init(); void exec(); void end(); };
class sRenderFbo : public Section { public:	sRenderFbo();  void load(); void init(); void exec(); void end(); };
class sObjectShader : public Section { public:	sObjectShader();  void load(); void init(); void exec(); void end(); };
class sGLSLShaderBind : public Section { public: sGLSLShaderBind(); void load(); void init(); void exec(); void end(); };
class sGLSLShaderUnbind : public Section { public: sGLSLShaderUnbind(); void load(); void init(); void exec(); void end(); };

#endif