// Cubemap.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include <string>

namespace Phoenix {

	class Cubemap {
	public:
		std::vector<std::string> filename;
		std::vector<int> width;
		std::vector<int> height;
		float mem;
		GLuint cubemapID;

		Cubemap();
		virtual ~Cubemap();

		bool load(std::vector<std::string> faces, bool flip);
		void bind(GLuint TexUnit = 0) const;

	private:

	};
}