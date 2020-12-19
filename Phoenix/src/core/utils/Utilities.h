// Utilities.h
// Spontz Demogroup

#pragma once

#include "main.h"

#include <iostream>
#include <string>
#include <time.h>

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ZERO_MEM_VAR(var) memset(&var, 0, sizeof(var))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
#define SAFE_DELETE_ARRAY(p) if (p) { delete[] p; p = NULL; }

namespace Util {
	const double CurrentTime();
	int getFloatVector(char *line, float *vector, int max);
	float getFloat(char *line);
	std::string& replaceString(std::string & subj, std::string old_str, std::string new_str);
	std::istream& safeGetline(std::istream& is, std::string& t);
}