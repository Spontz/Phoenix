// texture.cpp
// Spontz Demogroup

#include <stb_image.h>
#include <main.h>
#include "texture.h"

Texture::Texture()
{
	type = "texture_diffuse"; // default is set to diffuse texture
	use_linear = true;
	m_textureID = 0;
	m_mipmapLevels = 1;
	mem = 0;

}

Texture::~Texture()
{
	if (m_textureID != 0) {
		glDeleteTextures(1, &m_textureID);
		m_textureID = 0;
		m_mipmapLevels = 1;
		mem = 0;
	}
}

bool Texture::load(const std::string & file_name, bool flip)
{
	// If we already have loaded this texture, we unload it first
	if (m_textureID > 0) {
		glDeleteTextures(1, &m_textureID);
		m_textureID = 0;
		m_mipmapLevels = 1;
		mem = 0;
	}

	stbi_set_flip_vertically_on_load(flip); // required for loading textures properly

	filename = file_name;
	if (filename.empty())
		return false;

	bool is_loaded = true;

	unsigned char* data = stbi_load((filename).c_str(), &width, &height, &components, 0);

	if (data) {
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
			

		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
	
		m_mipmapLevels = (GLsizei)floor(log2(std::max(width, height)));
		if (m_mipmapLevels == 0)
			m_mipmapLevels = 1;
		glTextureStorage2D(m_textureID, m_mipmapLevels, internalFormat, width, height);

		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_mipmapLevels == 1 ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glTextureSubImage2D(m_textureID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
		if (m_mipmapLevels > 1)
			glGenerateTextureMipmap(m_textureID);
		
		if (m_mipmapLevels == 1)
			mem = (float)(width * height * components) / 1048576.0f;		// Calculate the texture mem (in mb)
		else
			mem = (float)(width * height * components * 1.33f) / 1048576.0f;		// Calculate the texture mem (in mb) for mipmaps

		// Unbind and restore pixel alignment
		glBindTexture(GL_TEXTURE_2D, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

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
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}