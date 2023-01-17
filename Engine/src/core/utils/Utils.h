#pragma once

#include "core/DemoKernel.h"

namespace Phoenix {

	class Utils final
	{

	public:

		static bool checkFileExists(std::string_view filename);										// Check if file exists
		static std::string readASCIIFile(std::string_view filename);								// Read an ASCII file contents
		static bool appendIntoASCIIFile(std::string_view filename, std::string_view message);		// Append message to file
	};

}
