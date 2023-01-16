#pragma once

#include "core/DemoKernel.h"

namespace Phoenix {

	class Utils final
	{
	public:
		static std::string readASCIIFile(std::string_view URI);
	};

}
