// utilities.h
// Spontz Demogroup

#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace Util {
	const std::string CurrentTime();
	unsigned long int getNextLine(const char *text, char *result, unsigned long int startPosition);
	void getKeyValue(char *line, char *key, char *value);
	int getFloatVector(char *line, float *vector, int max);
}



#endif
