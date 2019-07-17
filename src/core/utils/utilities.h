// utilities.h
// Spontz Demogroup

#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ZERO_MEM_VAR(var) memset(&var, 0, sizeof(var))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

namespace Util {
	const double CurrentTime();
	unsigned long int getNextLine(const char *text, char *result, unsigned long int startPosition);
	void getKeyValue(char *line, char *key, char *value);
	int getFloatVector(char *line, float *vector, int max);
	float getFloat(char *line);
	std::string& replaceString(std::string & subj, std::string old_str, std::string new_str);
}



#endif
