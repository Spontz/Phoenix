// Cubemap.cpp
// Spontz Demogroup

#include <stb_image.h>
#include <main.h>
#include "core/renderer/Cubemap.h"

namespace Phoenix {

	Cubemap::Cubemap()
	{
		cubemapID = 0;
		mem = 0;
	}

	Cubemap::~Cubemap()
	{
		if (cubemapID != 0) {
			glDeleteTextures(1, &cubemapID);
			width.clear();
			height.clear();
			cubemapID = 0;
			mem = 0;
		}
	}

	bool Cubemap::load(std::vector<std::string> faces_file_name, bool flip)
	{
		// If we already have loaded this cubemap, we unload it first
		if (cubemapID > 0) {
			glGenTextures(1, &cubemapID);
			cubemapID = 0;
			mem = 0;
		}

		bool is_loaded = true;

		stbi_set_flip_vertically_on_load(flip); // required for loading textures properly

		glGenTextures(1, &cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		int Width, Height, components;
		for (unsigned int i = 0; i < faces_file_name.size(); i++)
		{
			filename.push_back(faces_file_name[i]);
			unsigned char* data = stbi_load(faces_file_name[i].c_str(), &Width, &Height, &components, 0);
			this->width.push_back(Width);
			this->height.push_back(Height);
			if (data)
			{
				GLenum internalFormat = 0;
				GLenum dataFormat = 0;
				if (components == 1) {
					internalFormat = GL_R8;
					dataFormat = GL_RED;
				}
				else if (components == 2) {
					internalFormat = GL_RG8;
					dataFormat = GL_RG;
				}
				else if (components == 3) {
					internalFormat = GL_RGB8;
					dataFormat = GL_RGB;
				}
				else if (components == 4) {
					internalFormat = GL_RGBA8;
					dataFormat = GL_RGBA;
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, Width, Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			}

			else {
				Logger::error("Failed loading cubemap from file: %s", faces_file_name[i].c_str());
				is_loaded = false;
			}
			mem += (float)(Width * Height * 3) / 1048576.0f;		// Calculate the texture mem (in mb)
			stbi_image_free(data);
		}
		// Check if the cubemap images sizes are OK
		for (unsigned int i = 0; i < faces_file_name.size(); i++)
		{
			if ((this->width[0] != this->width[i]) || (this->height[0] != this->height[i])) {
				Logger::error("The cubemap image %s has a different size from the first one. Please check that all cubemap images have same size", faces_file_name[i].c_str());
				is_loaded = false;
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		stbi_set_flip_vertically_on_load(false); // Set always to "false"
		return is_loaded;
	}

	void Cubemap::bind(GLuint TexUnit) const
	{
		glBindTextureUnit(TexUnit, cubemapID);
	}
}