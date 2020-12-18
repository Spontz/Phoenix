// Texture.h
// Spontz Demogroup

#pragma once

#include "main.h"

#include <string>

namespace Phoenix {

	// flags for texture properties
#define NO_MIPMAP 1
#define CLAMP_TO_EDGE 2
#define CLAMP 4
#define MODULATE 8
#define TEXTURE_1D 16

	class Texture {
	public:
		std::string	filename;
		std::string type;					// texture_diffuse / texture_specular / texture_normal / texture_height
		int			width, height, components;
		float		mem;						// Size in Mb
		bool		use_linear;
		GLuint		m_textureID;

		Texture();
		virtual ~Texture();

		bool load(const std::string& file_name, bool flip);
		bool loadFromMem(const unsigned char* data, int len, bool flip);
		bool keepData();	// Stores data texture in memory
		void freeData();	// Frees data texture from memory
		void bind(GLuint TexUnit = 0) const;
		glm::vec4 getColor(int x, int y);

	private:
		GLsizei m_mipmapLevels;
		unsigned char* textureData;

		void uploadtoGPU(const unsigned char* data);
	};
}