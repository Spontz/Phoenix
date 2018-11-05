// sections.h
// Spontz Demogroup

#ifndef SECTIONS_H
#define SECTIONS_H
#include <string>
#include <vector>
#include "core/section.h"
using namespace std;


class sLoading : public Section { public:	sLoading();  void preload(); void load(); void init(); void exec(); void end(); };
class sGLSLShaderBind : public Section { public: sGLSLShaderBind(); void preload(); void load(); void init(); void exec(); void end(); };
class sGLSLShaderUnbind : public Section { public: sGLSLShaderUnbind(); void preload(); void load(); void init(); void exec(); void end(); };
class sBackground		: public Section {};

#endif