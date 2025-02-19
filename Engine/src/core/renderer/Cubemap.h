// Cubemap.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include <string>

namespace Phoenix {

	class Cubemap;
	using SP_Cubemap = std::shared_ptr<Cubemap>;
	using WP_Cubemap = std::weak_ptr<Cubemap>;

	class Cubemap {
	public:
		std::vector<std::string>	m_filePath;
		std::vector<int>			m_width;
		std::vector<int>			m_height;
		float						m_mem;
		GLuint						m_cubemapID;

		Cubemap();
		virtual ~Cubemap();

		bool load(std::vector<std::string> faces, bool flip);
		void bind(GLuint TexUnit = 0) const;

	private:

	};
}