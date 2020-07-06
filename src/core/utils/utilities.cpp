// utilities.cpp
// Spontz Demogroup

#include "utilities.h"

const double Util::CurrentTime() {
	return glfwGetTime();
}

int Util::getFloatVector(char *line, float *vector, int max) {

	char result[256], *s;
	int chan, offset, n;

	chan = 0;
	offset = 0;
	do {
		// read separators
		s = &line[offset];
		while ((s[0] == '{') || (s[0] == ' ') || (s[0] == ',') || (s[0] == '\t')) {
			s++;
			offset++;
		}

		// read the float
		n = 0;
		while (((s[0] >= '0') && (s[0] <= '9')) || (s[0] == '.') || (s[0] == '-')) {
			result[n++] = s++[0];
			offset++;
		}
		result[n] = 0;

		// convert string to float
		if (n > 0) {
			if (chan >= max)
				return -1;// error("Script parser: too many floats in vector '%s'", line);
			sscanf(result, "%f", &vector[chan]);
			chan++;
		}

	} while (n > 0);

	return chan;
}

float Util::getFloat(char *line) {

	char result[256], *s;
	int offset, n;
	float value;

	
	offset = 0;
	do {
		// read separators
		s = &line[offset];
		while ((s[0] == '{') || (s[0] == ' ') || (s[0] == ',') || (s[0] == '\t')) {
			s++;
			offset++;
		}

		// read the float
		n = 0;
		while (((s[0] >= '0') && (s[0] <= '9')) || (s[0] == '.') || (s[0] == '-')) {
			result[n++] = s++[0];
			offset++;
		}
		result[n] = 0;

		// convert string to float
		if (n > 0) {
			sscanf(result, "%f", &value);
			return value;
		}

	} while (n > 0);

	return 0;
}



std::string& Util::replaceString(std::string & subj, std::string old_str, std::string new_str)
{
	size_t pos;
	do {
		pos = subj.find(old_str);
		if (pos != std::string::npos)
		{
			subj.erase(pos, old_str.size());
			subj.insert(pos, new_str);
		}
	} while (std::string::npos != pos);

	return subj;
}