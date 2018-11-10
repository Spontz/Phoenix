// texture.cpp
// Spontz Demogroup

#include <stb_image.h>
#include <main.h>
#include "texture.h"

Texture::Texture(): use_linear(true), to_id(0)
{
}

Texture::~Texture()
{
	if (to_id != 0) {
		glDeleteTextures(1, &to_id);
		to_id = 0;
	}
}

bool Texture::load(const std::string & file_name)
{
	filename = file_name;
	if (filename.empty())
		return false;

	bool is_loaded = false;

	unsigned char* pixels = stbi_load((filename).c_str(), &width, &height, &components, 4);

	if (pixels != nullptr) {
		glGenTextures(1, &to_id);
		glBindTexture(GL_TEXTURE_2D, to_id);

		glTexStorage2D(GL_TEXTURE_2D, 2 /* mip map levels */, GL_RGB8, width, height);
		glTexSubImage2D(GL_TEXTURE_2D, 0 /* mip map level */, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		is_loaded = true;
	}
	
	stbi_image_free(pixels);

	return is_loaded;
}

void Texture::bind(int index) const
{
	if (to_id != 0) {
			glActiveTexture(GL_TEXTURE0 + index);
			glBindTexture(GL_TEXTURE_2D, to_id);
	}
}
