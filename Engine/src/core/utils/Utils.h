#pragma once

#include "core/DemoKernel.h"

namespace Phoenix {

	class Utils final
	{

	public:

		static std::string readASCIIFile(std::string_view filename);
		static bool appendIntoASCIIFile(std::string_view filename, std::string_view message);
	};

}
