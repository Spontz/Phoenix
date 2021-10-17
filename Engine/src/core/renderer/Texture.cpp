// Texture.cpp
// Spontz Demogroup

#include <stb_image.h>
#include <main.h>
#include "core/renderer/Texture.h"

namespace Phoenix {

	Texture::Texture()
		:
		m_filename(""),
		m_width(-1),
		m_height(-1),
		m_components(-1),
		m_mem(0),
		m_textureID(0),
		m_mipmapLevels(0),
		m_textureData(nullptr)		
	{
		// Set default texture properties
		m_properties.m_flip = true;
		m_properties.m_type = "texture_diffuse";
		m_properties.m_useLinearFilter = true;
	}

	Texture::~Texture()
	{
		if (m_textureID != 0) {
			glDeleteTextures(1, &m_textureID);
			m_textureID = 0;
			m_mipmapLevels = 1;
			m_mem = 0;
		}
		freeData();
	}

	bool Texture::load(std::string_view const& file_name)
	{
		// If we already have loaded this texture, we unload it first
		if (m_textureID > 0) {
			glDeleteTextures(1, &m_textureID);
			m_textureID = 0;
			m_mipmapLevels = 1;
			m_mem = 0;
		}

		stbi_set_flip_vertically_on_load(m_properties.m_flip); // required for loading textures properly

		m_filename = file_name;
		if (m_filename.empty())
			return false;

		bool is_loaded = true;

		unsigned char* data = stbi_load((m_filename).c_str(), &m_width, &m_height, &m_components, 0);

		if (data) {
			uploadtoGPU(data);
		}
		else {
			Logger::error("Failed loading texture from file: {}", m_filename);
			is_loaded = false;
		}

		stbi_set_flip_vertically_on_load(false); // Set always to "false"
		stbi_image_free(data);

		return is_loaded;
	}

	bool Texture::loadFromMem(const unsigned char* data, int len)
	{
		// If we already have loaded this texture, we unload it first
		if (m_textureID > 0) {
			glDeleteTextures(1, &m_textureID);
			m_textureID = 0;
			m_mipmapLevels = 1;
			m_mem = 0;
		}

		stbi_set_flip_vertically_on_load(m_properties.m_flip); // required for loading textures properly

		m_filename = "Embedded texture";
		bool is_loaded = true;

		unsigned char* data_stbi = stbi_load_from_memory(data, len, &m_width, &m_height, &m_components, 0);

		if (data_stbi) {
			uploadtoGPU(data_stbi);
		}
		else {
			Logger::error("Failed loading embedded texture");
			is_loaded = false;
		}

		stbi_set_flip_vertically_on_load(false); // Set always to "false"
		stbi_image_free(data_stbi);

		return is_loaded;
	}

	bool Texture::keepData()
	{
		freeData();
		m_textureData = stbi_load((m_filename).c_str(), &m_width, &m_height, &m_components, 0);
		if (m_textureData)
			return true;
		else
			return false;
	}

	void Texture::freeData()
	{
		if (m_textureData)
			stbi_image_free(m_textureData);
		m_textureData = nullptr;
	}

	void Texture::bind(GLuint TexUnit) const
	{
		glBindTextureUnit(TexUnit, m_textureID);
	}

	glm::vec4 Texture::getColor(int x, int y)
	{
		if (!m_textureData || x < 0 || x >= m_width || y < 0 || y >= m_height)
			return glm::vec4(0);
		else
		{
			// Hack: stb_image stores data flipped
			int coord_x = x;// width - x;
			int coord_y = m_height - y;
			unsigned bytePerPixel = m_components;
			unsigned char* pixelOffset = m_textureData + (coord_x + m_width * coord_y) * bytePerPixel;
			unsigned char r = pixelOffset[0];
			unsigned char g = pixelOffset[1];
			unsigned char b = pixelOffset[2];
			unsigned char a = m_components >= 4 ? pixelOffset[3] : 0xff;
			glm::vec4 color = glm::vec4(static_cast<float>(r) / 255.0f,
				static_cast<float>(g) / 255.0f,
				static_cast<float>(b) / 255.0f,
				static_cast<float>(a) / 255.0f);
			return color;
		}
	}

	void Texture::uploadtoGPU(const unsigned char* data)
	{
		GLenum internalFormat = 0;
		GLenum dataFormat = 0;

		switch (m_components) {
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

		glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


		m_mipmapLevels = (GLsizei)floor(log2(std::max(m_width, m_height)));
		if (m_mipmapLevels == 0)
			m_mipmapLevels = 1;
		glTextureStorage2D(m_textureID, m_mipmapLevels, internalFormat, m_width, m_height);

		if (m_properties.m_useLinearFilter) {
			glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_mipmapLevels == 1 ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else {
			glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, m_mipmapLevels == 1 ? GL_NEAREST : GL_LINEAR_MIPMAP_NEAREST);
			glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		

		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_textureID, 0, 0, 0, m_width, m_height, dataFormat, GL_UNSIGNED_BYTE, data);
		if (m_mipmapLevels > 1)
			glGenerateTextureMipmap(m_textureID);

		if (m_mipmapLevels == 1)
			m_mem = (float)(m_width * m_height * m_components) / 1048576.0f;		// Calculate the texture mem (in mb)
		else
			m_mem = (float)(m_width * m_height * m_components * 1.33f) / 1048576.0f;		// Calculate the texture mem (in mb) for mipmaps

		// Unbind and restore pixel alignment
		glBindTexture(GL_TEXTURE_2D, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}
}
