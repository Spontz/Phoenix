// texture.cpp
// Spontz Demogroup

#include <stb_image.h>
#include <main.h>
#include "texture.h"

Texture::Texture()
{
	type = "texture_diffuse"; // default is set to diffuse texture
	shaderName = "texture_diffuse";
	use_linear = true;
	textureID = 0;
	mem = 0;

}

Texture::~Texture()
{
	if (textureID != 0) {
		glDeleteTextures(1, &textureID);
		textureID = 0;
		mem = 0;
	}
}

bool Texture::load(const std::string & file_name, bool flip)
{
	// If we already have loaded this texture, we unload it first
	if (textureID > 0) {
		glDeleteTextures(1, &textureID);
		textureID = 0;
		mem = 0;
	}

	stbi_set_flip_vertically_on_load(flip); // required for loading textures properly

	filename = file_name;
	if (filename.empty())
		return false;

	bool is_loaded = true;

	unsigned char* data = stbi_load((filename).c_str(), &width, &height, &components, 0);

	if (data) {
		GLenum format;
		if (components == 1)
			format = GL_RED;
		else if (components == 2)
			format = GL_RG;
		else if (components == 3)
			format = GL_RGB;
		else if (components == 4)
			format = GL_RGBA;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // We use GL_CLAMP_TO_EDGE un RGBA textures to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // Be careful! in some 3d models they don't look good with GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		mem = (float)(width * height * components) / 1048576.0f;		// Calculate the texture mem (in mb)
	}
	else {
		LOG->Error("Failed loading texture from file: %s", filename.c_str());
		is_loaded = false;
	}

	stbi_set_flip_vertically_on_load(false); // Set always to "false"
	stbi_image_free(data);

	return is_loaded;
}

void Texture::active(int TexUnit) const
{
	glActiveTexture(GL_TEXTURE0 + TexUnit);
}

void Texture::bind(int TexUnit) const
{
	glActiveTexture(GL_TEXTURE0 + TexUnit);
	glBindTexture(GL_TEXTURE_2D, textureID);
}