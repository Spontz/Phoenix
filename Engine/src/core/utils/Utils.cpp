// Utils.cpp
// Spontz Demogroup

#include "main.h"
#include "Utils.h"
#include <iomanip>
#include <filesystem>
#include <io.h>

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

	std::vector<std::string> Utils::getFilepathsFromFolder(std::string folderPath, std::string extension)
	{
		std::vector<std::string> fileName;

		struct _finddata_t FindData;
		intptr_t hFile;
		std::string fullpath;
		std::string ScriptRelativePath;

		fullpath = folderPath + "/*." + extension;
		Logger::info(LogLevel::med, "Scanning folder: {}", fullpath);
		if ((hFile = _findfirst(fullpath.c_str(), &FindData)) != -1L) {
			do {
				ScriptRelativePath = folderPath + "/" + FindData.name;
				fileName.push_back(ScriptRelativePath);
			} while (_findnext(hFile, &FindData) == 0);
			_findclose(hFile);
		}
		else {
			Logger::error("No files not found in {} folder", fullpath);
		}

		for (const auto& file : fileName) {
			Logger::info(LogLevel::low, "File found: {}", file.c_str());
		}


		return fileName;
	}

	// Random number generation: Credits: TheCherno (https://www.youtube.com/watch?v=5_RAHZQCPjE)
	uint32_t Utils::PCG_Hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	float Utils::randomFloat(uint32_t& seed)
	{
		seed = PCG_Hash(seed);
		return static_cast<float>(seed) / static_cast<float>(std::numeric_limits<uint32_t>::max());
	}

	glm::vec3 Utils::randomVec3(uint32_t& seed)
	{
		return glm::vec3(randomFloat(seed), randomFloat(seed), randomFloat(seed));
	}

	glm::vec3 Utils::randomVec3(uint32_t& seed, float min, float max)
	{
		return glm::vec3(randomFloat(seed) * (max-min) + min, randomFloat(seed) * (max - min) + min, randomFloat(seed) * (max - min) + min);
	}

	glm::vec3 Utils::randomVec3_05(uint32_t& seed)
	{
		return glm::vec3(randomFloat(seed) - 0.5f , randomFloat(seed) - 0.5f, randomFloat(seed) - 0.5f);
	}
}
