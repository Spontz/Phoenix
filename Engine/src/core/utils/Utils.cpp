// Utils.cpp
// Spontz Demogroup

#include "main.h"
#include "Utils.h"
#include <iomanip>

namespace Phoenix {

	std::string Utils::readASCIIFile(std::string_view URI)
	{
		std::string result;
		std::ifstream file(URI.data(), std::ios::in | std::ios::binary);
		if (file.good()) {
			file.seekg(0, std::ios::end);
			size_t size = file.tellg();
			if (size != -1) {
				result.resize(size);
				file.seekg(0, std::ios::beg);
				file.read(&result[0], size);
				file.close();
			}
			else {
				Logger::error("Could not read file: {}", URI);
			}
		}
		else {
			Logger::error("Could not open file: {}", URI);
		}
		return result;

		/*
		std::string result;
		std::ifstream file(URI.data(), std::ios::in | std::ios::binary);
		if (file.good()) {
			result = { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
			file.close();
		}
		else
			Logger::error("Could not read file: {}", URI);

		return result;
		*/
	}
}
