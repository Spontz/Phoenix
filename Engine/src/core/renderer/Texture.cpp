// Texture.cpp
// Spontz Demogroup

#include <stb_image.h>
#include <main.h>
#include "core/renderer/Texture.h"

namespace Phoenix {

	Texture::Texture()
		:
		m_filePath(""),
		m_width(-1),
		m_height(-1),
		m_components(-1),
		m_depth(-1),
		m_type(TextureType::INVALID),
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
		clear();
	}

	// TEXTURE2D / From disk
	bool Texture::load(std::string_view const& path)
	{
		// If we already have loaded this texture, we unload it first
		clear();

		stbi_set_flip_vertically_on_load(m_properties.m_flip); // required for loading textures properly

		m_filePath = path;
		m_type = TextureType::SAMPLER2D;
		m_depth = 1;

		if (m_filePath.empty())
			return false;

		bool is_loaded = true;

		unsigned char* data = stbi_load((m_filePath).c_str(), &m_width, &m_height, &m_components, 0);

		if (data) {
			upload2DtoGPU(data);
		}
		else {
			Logger::error("Failed loading texture from file: {}", m_filePath);
			is_loaded = false;
		}

		stbi_set_flip_vertically_on_load(false); // Set always to "false"
		stbi_image_free(data);

		return is_loaded;
	}

	// TEXTURE2D / From memory
	bool Texture::loadFromMem(const unsigned char* data, int len)
	{
		// If we already have loaded this texture, we unload it first
		clear();

		stbi_set_flip_vertically_on_load(m_properties.m_flip); // required for loading textures properly

		m_filePath = "Embedded texture";
		m_type = TextureType::SAMPLER2D;
		m_depth = 1;

		bool is_loaded = true;

		unsigned char* data_stbi = stbi_load_from_memory(data, len, &m_width, &m_height, &m_components, 0);

		if (data_stbi) {
			upload2DtoGPU(data_stbi);
		}
		else {
			Logger::error("Failed loading embedded texture");
			is_loaded = false;
		}

		stbi_set_flip_vertically_on_load(false); // Set always to "false"
		stbi_image_free(data_stbi);

		return is_loaded;
	}

	// TEXTURE3D / From disk
	bool Texture::load(std::vector<std::string> path)
	{
		// If we already have loaded this texture, we unload it first
		clear();

		stbi_set_flip_vertically_on_load(m_properties.m_flip); // required for loading textures properly

		m_filesPath = path;
		m_type = TextureType::SAMPLER3D;
		m_depth = static_cast<int>(m_filesPath.size());

		if (m_filesPath.empty())
			return false;

		bool is_loaded = true;

		// Use the first texture to get Width, Height, components, internalFormat and dataFormat
		int Width, Height, components;
		// Allocate memory for all the textures and loads it
		unsigned char* allData = NULL;

		if (getImageProperties(m_filesPath[0].c_str(), &Width, &Height, &components)) {
			m_width = Width;
			m_height = Height;
			m_components = components;

			// Allocate memory for all the textures and loads it
			allData = new unsigned char[m_width * m_height * m_components * m_depth]();
			
			// Load all files into memory
			for (unsigned int i = 0; i < m_filesPath.size(); ++i) {
				unsigned char* data = stbi_load(m_filesPath[i].c_str(), &Width, &Height, &components, 0);
				if (data) {
					// Chekc if the loaded file is equal in properties as the first one
					if (Width != m_width || Height != m_height || components != m_components) {
						Logger::error("Failed loading 3D texture, format (width, height or components) are not equal. Error found in file: {}", m_filesPath[i]);
						is_loaded = false;
						delete[] allData;
						break;
					}
					// Copy the image data to the allData array
					memcpy(allData + (m_width * m_height * m_components * i), data, m_width * m_height * m_components);
					stbi_image_free(data);
				}
				else {
					Logger::error("Failed loading texture from file: {}", m_filesPath[i]);
					delete[] allData;
					is_loaded = false;
					break;
				}
			}
		}
		else {
			Logger::error("Failed loading texture from file: {}", m_filesPath[0]);
			is_loaded = false;
		}

		if (is_loaded) {
			upload3DtoGPU(allData);
		}

		stbi_set_flip_vertically_on_load(false); // Set always to "false"
		if (allData)
			delete[] allData;

		return is_loaded;
	}


	bool Texture::keepData()
	{
		if (m_filePath.empty())
			return false;

		freeData();
		m_textureData = stbi_load((m_filePath).c_str(), &m_width, &m_height, &m_components, 0);
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
			int coord_y = (m_height-1) - y;
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

	void Texture::clear()
	{
		if (m_textureID != 0) {
			glDeleteTextures(1, &m_textureID);
		
			m_filePath = "";
			m_filesPath.clear();
			m_width = -1;
			m_height = -1;
			m_components = -1;
			m_depth = -1;
			m_type = TextureType::INVALID;
			m_mem = 0;
			m_mipmapLevels = 0;
		}
		freeData();
	}

	void Texture::upload2DtoGPU(const unsigned char* data)
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

	void Texture::upload3DtoGPU(const unsigned char* data)
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

		glCreateTextures(GL_TEXTURE_3D, 1, &m_textureID);
		glBindTexture(GL_TEXTURE_3D, m_textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


		m_mipmapLevels = (GLsizei)floor(log2(std::max(m_width, m_height)));
		if (m_mipmapLevels == 0)
			m_mipmapLevels = 1;
		glTextureStorage3D(m_textureID, m_mipmapLevels, internalFormat, m_width, m_height, m_depth);

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
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_R, GL_REPEAT);

		glTextureSubImage3D(m_textureID, 0, 0, 0,0, m_width, m_height, m_depth, dataFormat, GL_UNSIGNED_BYTE, data);
		if (m_mipmapLevels > 1)
			glGenerateTextureMipmap(m_textureID);

		if (m_mipmapLevels == 1)
			m_mem = (float)(m_width * m_height * m_components * m_depth) / 1048576.0f;		// Calculate the texture mem (in mb)
		else
			m_mem = (float)(m_width * m_height * m_components * m_depth * 1.33f) / 1048576.0f;		// Calculate the texture mem (in mb) for mipmaps

		// Unbind and restore pixel alignment
		glBindTexture(GL_TEXTURE_3D, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

	bool Texture::getImageProperties(char const* path, int* width, int* height, int* channels)
	{
		stbi_uc* data;
		data = stbi_load(path, width, height, channels, 0);
		if (data) {
			stbi_image_free(data);
			return true;
		}
		else {
			return false;
		}
	}
}
