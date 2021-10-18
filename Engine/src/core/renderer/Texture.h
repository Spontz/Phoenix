// Texture.h
// Spontz Demogroup

#pragma once

#include "main.h"

#include <string>

namespace Phoenix {

	class Texture;
	using SP_Texture = std::shared_ptr<Texture>;
	using WP_Texture = std::weak_ptr<Texture>;

	class Texture {
	public:
		struct Properties final {
			bool				m_flip = true;
			bool				m_useLinearFilter = true;
			std::string_view	m_type = "texture_diffuse";
		};

	public:
		std::string	m_filename;
		int			m_width;
		int			m_height;
		int			m_components;
		float		m_mem;						// Size in Mb
		GLuint		m_textureID;
		Properties	m_properties;				// Texture properties

		Texture();
		virtual ~Texture();

		bool load(std::string_view const& file_name);
		bool loadFromMem(const unsigned char* data, int len);
		bool keepData();	// Stores data texture in memory
		void freeData();	// Frees data texture from memory
		void bind(GLuint TexUnit = 0) const;
		glm::vec4 getColor(int x, int y);

	private:
		GLsizei			m_mipmapLevels;
		unsigned char*	m_textureData;

		void uploadtoGPU(const unsigned char* data);
	};
}