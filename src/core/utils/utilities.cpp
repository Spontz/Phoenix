// utilities.cpp
// Spontz Demogroup

#include "utilities.h"

const std::string Util::CurrentTime() {
	float	now;
	char	buf[80];
	now = static_cast<float>(glfwGetTime());
	sprintf_s(buf, "%.4f", now);
	return buf;
}

// Given a text, this routine extracts the first line (Starting from the given character position)
// and puts it in the result variable. The function also returns the position in which the line was
// found to end, to allow the application the search of the next line. If there are not remaining lines
// in the psased string, we return 0
unsigned long int Util::getNextLine(const char *text, char *result, unsigned long int startPosition) {
	// Avoid an invalid startPosition
	if (startPosition > strlen(text)) {
		result = NULL;
		return 0;
	}

	// Start searching in the given position
	text = text + startPosition;

	if (text[0] == '\0') {
		// We are at the end of the string
		result = NULL;
		return 0;
	}
	else {
		// Populate the passed result variable with the line contents
		while (((text[0] != '\n') && (text[0] != '\r') && (text[0] != '\0'))) {
			result++[0] = text++[0];
			startPosition++;
		}
	}

	// Return the position in which we stopped the search. We add one because if not we will start looking
	// at the same position in which the search ended before (this is a loop we must avoid)
	return ++startPosition;
}

void Util::getKeyValue(char *line, char *key, char *value) {

	char *s = line;

	// read spaces or tabs
	while ((s[0] == ' ') || (s[0] == '\t')) {
		s++;
	}

	// get key
	while ((s[0] != ' ') && (s[0] != '\t')) {
		key++[0] = s++[0];
	}
	key[0] = 0;

	// read spaces or tabs
	while ((s[0] == ' ') || (s[0] == '\t')) {
		s++;
	}

	// get value
	while ((s[0] != 0) && ((s[0] != '\n') && (s[0] != '\r'))) {
		value++[0] = s++[0];
	}
	value[0] = 0;
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
			if (chan >= max) return 0;// error("Script parser: too many floats in vector '%s'", line);
			sscanf(result, "%f", &vector[chan]);
			chan++;
		}

	} while (n > 0);

	return chan;
}


