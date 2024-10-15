// Cubemap.cpp
// Spontz Demogroup

#include <stb_image.h>
#include <main.h>
#include "core/renderer/Cubemap.h"

namespace Phoenix {

	Cubemap::Cubemap()
	{
		m_cubemapID = 0;
		m_mem = 0;
	}

	Cubemap::~Cubemap()
	{
		if (m_cubemapID != 0) {
			glDeleteTextures(1, &m_cubemapID);
			m_filename.clear();
			m_width.clear();
			m_height.clear();
			m_cubemapID = 0;
			m_mem = 0;
		}
	}

	bool Cubemap::load(std::vector<std::string> faces_file_name, bool flip)
	{
		// If we already have loaded this cubemap, we unload it first
		if (m_cubemapID > 0) {
			glDeleteTextures(1, &m_cubemapID);
			m_filename.clear();
			m_width.clear();
			m_height.clear();
			m_cubemapID = 0;
			m_mem = 0;
		}

		bool is_loaded = true;

		stbi_set_flip_vertically_on_load(flip); // required for loading textures properly

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_cubemapID);
		
		int Width, Height, components;
		GLuint internalFormat = 0;
		GLuint dataFormat = 0;
		stbi_uc *data;

		// Use the first texture to get Width, Height, components, internalFormat and dataFormat
		// TODO: remove this to another function
		data = stbi_load(faces_file_name[0].c_str(), &Width, &Height, &components, 0);
		if (data) {
			switch (components) {
			case 1:
				internalFormat = GL_R8;
				dataFormat = GL_RED;
				break;
			case 2:
				internalFormat = GL_RG8;
				dataFormat = GL_RG;
				break;
			case 3:
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
				break;
			case 4:
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
				break;
			}
			stbi_image_free(data);
		}
		else {
			Logger::error("Failed loading cubemap from file: {}", faces_file_name[0]);
			is_loaded = false;
		}

		glTextureStorage2D(m_cubemapID, 1, internalFormat, Width, Height);
		for (unsigned int face = 0; face < faces_file_name.size(); ++face) {
			data = stbi_load(faces_file_name[face].c_str(), &Width, &Height, &components, 0);
			if (data) {
				m_filename.push_back(faces_file_name[face]);
				m_width.push_back(Width);
				m_height.push_back(Height);
				glTextureSubImage3D(m_cubemapID, 0, 0, 0,		// target, level , xOffset, yOffset
					face, Width, Height, 1, dataFormat, 		// cube map face, width, height, 1 face a time, format
					GL_UNSIGNED_BYTE, data);					// datatype, data pointer
				m_mem += (float)(Width * Height * components) / 1048576.0f;		// Calculate the texture mem (in mb)
				stbi_image_free(data);
				data = NULL;
			} else {
				Logger::error("Failed loading cubemap from file: {}", faces_file_name[face]);
				is_loaded = false;
			}
		}

		glTextureParameteri(m_cubemapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_cubemapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


		stbi_set_flip_vertically_on_load(false); // Set always to "false"
		return is_loaded;
	}

	void Cubemap::bind(GLuint TexUnit) const
	{
		glBindTextureUnit(TexUnit, m_cubemapID);
	}
}