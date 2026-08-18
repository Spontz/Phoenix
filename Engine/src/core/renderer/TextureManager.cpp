// TextureManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/TextureManager.h"

namespace {

	GLenum getRuntimeTexture3DFormat(std::string_view format)
	{
		if (format == "R16F")
			return GL_R16F;

		if (format == "R32F")
			return GL_R32F;

		if (format == "RGBA16F")
			return GL_RGBA16F;

		if (format == "RGBA32F")
			return GL_RGBA32F;

		return GL_NONE;
	}
}

namespace Phoenix {

	// Init vars
	TextureManager::TextureManager() {
		texture.clear();
		cubemap.clear();
		m_mem = 0;
		m_forceLoad = false;
	}

	TextureManager::~TextureManager()
	{
		clear();
	}

	SP_Texture TextureManager::addTexture(std::string_view const& path)
	{
		Texture::Properties props;	// Load default properties
		return addTexture(path, props);
	}

	SP_Texture TextureManager::addTexture(std::string_view const& path, Texture::Properties& texProperties) {
		unsigned int i;
		SP_Texture p_tex;

		// check if texture is already loaded, then we just retrieve the ID of our texture
		for (i = 0; i < texture.size(); i++) {
			if (texture[i]->m_filePath.compare(path) == 0) {
				p_tex = texture[i];
			}
		}

		if (p_tex == nullptr) { // If the texture has not been found, we need to load it for the first time
			SP_Texture new_tex = std::make_shared<Texture>();
			new_tex->m_properties = texProperties;
			if (new_tex->load(path)) {
				texture.emplace_back(new_tex);
				m_mem += new_tex->m_mem;
				p_tex = new_tex;
				Logger::info(LogLevel::med, "Texture {} [id: {}] loaded OK. Overall texture Memory: {:.3f}Mb", path, texture.size() - 1, m_mem);
			}
			else {
				Logger::error("Could not load texture: {}", path);
			}

		}
		else { // If the texture is catched we should not do anything, unless we have been told to upload it again
			if (m_forceLoad) {
				m_mem -= p_tex->m_mem; // Decrease the overall texture memory
				p_tex->m_properties = texProperties;
				if (p_tex->load(path)) {
					m_mem += p_tex->m_mem;
					Logger::info(LogLevel::med, "Texture {} [id: {}] force reload OK. Overall texture Memory: {:.3f}Mb", path, i, m_mem);
				}
				else
					Logger::error("Could not load texture: {}", path);
			}
		}

		return p_tex;
	}

	SP_Texture TextureManager::addTexture(std::vector<std::string> path, Texture::Properties& texProperties)
	{
		unsigned int i;
		SP_Texture p_tex;

		// check if the 3D texture is already loaded, then we just return the ID of our texture
		for (i = 0; i < texture.size(); i++) {
			if (texture[i]->m_filesPath == path) {
				p_tex = texture[i];
			}
		}

		if (p_tex == nullptr) { // If the texture has not been found, we need to load it for the first time
			SP_Texture new_tex = std::make_shared<Texture>();
			new_tex->m_properties = texProperties;
			if (new_tex->load(path)) {
				texture.emplace_back(new_tex);
				m_mem += new_tex->m_mem;
				p_tex = new_tex;
				Logger::info(LogLevel::med, "Texture {} [id: {}] loaded OK. Overall texture Memory: {:.3f}Mb", path, texture.size() - 1, m_mem);
			}
			else {
				Logger::error("Could not load texture: {}", path);
			}

		}
		else { // If the texture is catched we should not do anything, unless we have been told to upload it again
			if (m_forceLoad) {
				m_mem -= p_tex->m_mem; // Decrease the overall texture memory
				p_tex->m_properties = texProperties;
				if (p_tex->load(path)) {
					m_mem += p_tex->m_mem;
					Logger::info(LogLevel::med, "Texture {} [id: {}] force reload OK. Overall texture Memory: {:.3f}Mb", path, i, m_mem);
				}
				else
					Logger::error("Could not load texture: {}", path);
			}
		}

		return p_tex;
	}

	SP_Texture TextureManager::addTextureFromMem(const unsigned char* data, int32_t len, Texture::Properties& texProperties)
	{
		SP_Texture p_tex;

		auto new_tex = std::make_shared<Texture>();
		new_tex->m_properties = texProperties;
		if (new_tex->loadFromMem(data, len)) {
			texture.emplace_back(new_tex);
			m_mem += new_tex->m_mem;
			p_tex = new_tex;
			Logger::info(LogLevel::med, "Texture embedded [id: {}] loaded OK. Overall texture Memory: {:.3f}Mb", texture.size() - 1, m_mem);
		}
		else {
			Logger::error("Could not load embedded texture");
		}

		return p_tex;
	}

	// Adds a Cubemap into the queue, returns the Number of the cubemap added
	SP_Cubemap TextureManager::addCubemap(std::vector<std::string> const& path, bool flip)
	{
		unsigned int i;
		SP_Cubemap p_cubemap;

		// check if cubemap is already loaded, then we just return the ID of our texture
		for (i = 0; i < cubemap.size(); i++) {
			if (cubemap[i]->m_filePath == path) // Check if all the paths are the same (so cubemap is already loaded)
			{
				p_cubemap = cubemap[i];
			}
		}

		if (p_cubemap == nullptr) { // If the cubemap has not been found, we need to load it from the first time
			SP_Cubemap new_cubemap = std::make_shared<Cubemap>();
			if (new_cubemap->load(path, flip)) {
				cubemap.emplace_back(new_cubemap);
				p_cubemap = new_cubemap;
				m_mem += new_cubemap->m_mem;
				Logger::info(LogLevel::med, "Cubemap {} [id: {}] loaded OK. Overall texture Memory: {:.3f}Mb", path[0], cubemap.size() - 1, m_mem);
			}
			else {
				for (i = 0; i < path.size(); i++)
					Logger::error("Could not load cubemap, check these files: {}", path[i]);
			}
		}
		else { // If the cubemap is catched we should not do anything, unless we have been told to upload it again
			if (m_forceLoad) {
				m_mem -= p_cubemap->m_mem; // Decrease the overall texture memory
				if (p_cubemap->load(path, flip)) {
					m_mem += p_cubemap->m_mem;
					Logger::info(LogLevel::med, "Cubemap {} [id:{}] force reload OK. Overall texture Memory : {:.3f}Mb", path[0], i, m_mem);
				}
				else {
					for (i = 0; i < path.size(); i++)
						Logger::error("Could not load cubemap, check these files: {}", path[i]);
				}
			}
		}

		return p_cubemap;
	}

	int TextureManager::getOpenGLTextureID(int32_t index)
	{
		if (index < texture.size())
			return texture[index]->m_textureID;
		return -1;
	}

	bool TextureManager::initRuntime3D(int count, int width, int height, int depth, std::string_view format, bool useLinearFilter)
	{
		runtime3D.clear();

		if (count <= 0) {
			Logger::info(LogLevel::low, "Runtime 3D texture pool disabled");
			return true;
		}

		if (width <= 0 || height <= 0 || depth <= 0) {
			Logger::error("Invalid runtime 3D texture dimensions: {}x{}x{}", width, height, depth);
			return false;
		}

		const GLenum internalFormat = getRuntimeTexture3DFormat(format);

		if (internalFormat == GL_NONE) {
			Logger::error("Unknown runtime 3D texture format: {}", format);
			return false;
		}

		runtime3D.reserve(count);

		float runtimeMemory = 0.0f;

		for (int i = 0; i < count; ++i) {
			auto tex = std::make_shared<Texture>();
			
			if (!tex->create3D(width, height, depth, internalFormat, useLinearFilter)) {
				Logger::error("Could not create runtime 3D texture {}", i);
				runtime3D.clear();
				return false;
			}

			runtimeMemory += tex->m_mem;
			runtime3D.emplace_back(tex);
		}

		m_mem += runtimeMemory;
		Logger::info(LogLevel::med, "Runtime 3D texture pool created: {} textures, {}x{}x{}, format {}, {:.3f} Mb", count, width, height, depth, format, runtimeMemory);
		return true;
	}

	SP_Texture TextureManager::getRuntime3D(int index) const
	{
		if (index < 0 || static_cast<size_t>(index) >= runtime3D.size()) {
			Logger::error("Runtime 3D texture index out of range: {}", index);
			return nullptr;
		}

		return runtime3D[static_cast<size_t>(index)];
	}

	void TextureManager::initTextureStates()
	{
		// Bind no texture in texUnit 0
		//glBindTextureUnit(0, 0); --> TODO: This gives error on some graphics card (https://community.intel.com/t5/Graphics/intel-uhd-graphics-630-with-latest-driver-will-cause-error-when/td-p/1161376)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void TextureManager::clear()
	{
		texture.clear();
		cubemap.clear();
		runtime3D.clear();
		m_mem = 0;
	}
}
