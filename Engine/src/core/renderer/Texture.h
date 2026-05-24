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

		enum TextureType {
			INVALID = 0,
			SAMPLER1D,
			SAMPLER2D,
			SAMPLER3D			
		};

	public:

		struct Properties final {
			bool				m_flip = true;
			bool				m_useLinearFilter = true;
			std::string			m_type = "texture_diffuse";
		};

	public:
		std::string					m_filePath;					// For 2D textures
		std::vector<std::string>	m_filesPath;				// For 3D textures
		int							m_width;
		int							m_height;
		int							m_depth;
		int							m_components;
		float						m_mem;						// Size in Mb
		TextureType					m_type;
		GLuint						m_textureID;
		Properties					m_properties;				// Texture properties

		Texture();
		virtual ~Texture();

		bool load(std::string_view const& path);		// Loads a 2D Texture
		bool load(std::vector<std::string> path);		// Loads a 3D Texture
		bool loadFromMem(const unsigned char* data, int len);
		bool keepData();	// Stores data texture in memory
		void freeData();	// Frees data texture from memory
		void bind(GLuint TexUnit = 0) const;
		glm::vec4 getColor(int x, int y);

	private:
		GLsizei			m_mipmapLevels;
		unsigned char*	m_textureData;

		void clear();
		void upload2DtoGPU(const unsigned char* data);	// Upload 2D texture to GPU
		void upload3DtoGPU(const unsigned char* data);	// Upload 3D texture to GPU
		bool getImageProperties(char const* path, int* width, int* height, int* channels);
	};
}