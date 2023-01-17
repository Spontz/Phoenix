// Utils.cpp
// Spontz Demogroup

#include "main.h"
#include "Utils.h"
#include <iomanip>
#include <filesystem>

namespace Phoenix {

	bool Utils::checkFileExists(std::string_view fileName) {
		return std::filesystem::exists(fileName);
	}

	std::string Utils::readASCIIFile(std::string_view fileName) 
	{
		std::string result;
		std::ifstream file(fileName.data(), std::ios::in | std::ios::binary);
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
				Logger::error("Could not read file: {}", fileName);
			}
		}
		else {
			Logger::error("Could not open file: {}", fileName);
		}
		return result;

		/*
		std::string result;
		std::ifstream file(fileName.data(), std::ios::in | std::ios::binary);
		if (file.good()) {
			result = { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
			file.close();
		}
		else
			Logger::error("Could not read file: {}", fileName);

		return result;
		*/
	}

	bool Utils::appendIntoASCIIFile(std::string_view fileName, std::string_view message)
	{
		bool saved = false;

		// write into file
		std::ofstream file;

		file.open(fileName.data(), std::ios::out | std::ios::app);
		if (file.is_open())
		{
			file << message;
			file.close();
			saved = true;
		}
		return saved;
	}
}
