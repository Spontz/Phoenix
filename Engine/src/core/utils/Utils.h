#pragma once

#include "core/DemoKernel.h"

namespace Phoenix {

	class Utils final
	{

	public:

		static bool checkFileExists(std::string_view filename);										// Check if file exists
		static std::string readASCIIFile(std::string_view filename);								// Read an ASCII file contents
		static bool appendIntoASCIIFile(std::string_view filename, std::string_view message);		// Append message to file

		// Random number generation
	private:
		static uint32_t PCG_Hash(uint32_t input);
	public:
		static float randomFloat(uint32_t& seed);							// return random float between 0 and 1
		static glm::vec3 randomVec3(uint32_t& seed);						// return random vec3 between 0 and 1
		static glm::vec3 randomVec3(uint32_t& seed, float min, float max);	// return random vec3 between min and max
		static glm::vec3 randomVec3_05(uint32_t& seed);						// return random vec3 between -0.5 and 0.5
	};

}
