// TextureManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/TextureManager.h"

namespace Phoenix {

	// Init vars
	TextureManager::TextureManager() {
		texture.clear();
		cubemap.clear();
		mem = 0;
		forceLoad = false;
	}

	TextureManager::~TextureManager()
	{
		clear();
	}

	SP_Texture TextureManager::addTexture(std::string_view const& path, bool flip, std::string_view const& type) {
		unsigned int i;
		SP_Texture p_tex;

		// check if texture is already loaded, then we just retrieve the ID of our texture
		for (i = 0; i < texture.size(); i++) {
			if (texture[i]->filename.compare(path) == 0) {
				p_tex = texture[i];
			}
		}

		if (p_tex == nullptr) { // If the texture has not been found, we need to load it for the first time
			SP_Texture new_tex = std::make_shared<Texture>();
			if (new_tex->load(path, flip)) {
				new_tex->type = type;
				texture.push_back(new_tex);
				mem += new_tex->mem;
				p_tex = new_tex;
				Logger::info(LogLevel::med, "Texture %s [id: %d] loaded OK. Overall texture Memory: %.3fMb", path.data(), texture.size() - 1, mem);
			}
			else {
				Logger::error("Could not load texture: %s", path.data());
			}

		}
		else { // If the texture is catched we should not do anything, unless we have been told to upload it again
			if (forceLoad) {
				mem -= p_tex->mem; // Decrease the overall texture memory
				if (p_tex->load(path, flip)) {
					p_tex->type = type;
					mem += p_tex->mem;
					Logger::info(LogLevel::med, "Texture %s [id: %d] force reload OK. Overall texture Memory: %.3fMb", path.data(), i, mem);
				}
				else
					Logger::error("Could not load texture: %s", path.data());
			}
		}

		return p_tex;
	}

	SP_Texture TextureManager::addTextureFromMem(const unsigned char* data, int len, bool flip, std::string_view const& type) {
		SP_Texture p_tex;

		auto new_tex = std::make_shared<Texture>();
		if (new_tex->loadFromMem(data, len, flip)) {
			new_tex->type = type;
			texture.push_back(new_tex);
			mem += new_tex->mem;
			p_tex = new_tex;
			Logger::info(LogLevel::med, "Texture embedded [id: %d] loaded OK. Overall texture Memory: %.3fMb", texture.size() - 1, mem);
		}
		else {
			Logger::error("Could not load embedded texture");
		}

		return p_tex;
	}

	// Adds a Cubemap into the queue, returns the Number of the cubemap added
	Cubemap* TextureManager::addCubemap(std::vector<std::string> const& path, bool flip)
	{
		unsigned int i;
		Cubemap* p_cubemap = nullptr;

		// check if cubemap is already loaded, then we just return the ID of our texture
		bool already_loaded = true;
		for (i = 0; i < cubemap.size(); i++) {
			if (cubemap[i]->filename == path) // Check if all the paths are the same (so cubemap is already loaded)
			{
				p_cubemap = cubemap[i];
			}
		}

		if (p_cubemap == nullptr) { // If the cubemap has not been found, we need to load it from the first time
			Cubemap* new_cubemap = new Cubemap();
			if (new_cubemap->load(path, flip)) {
				cubemap.push_back(new_cubemap);
				p_cubemap = new_cubemap;
				mem += new_cubemap->mem;
				Logger::info(LogLevel::med, "Cubemap %s [id: %d] loaded OK. Overall texture Memory: %.3fMb", path[0].c_str(), cubemap.size() - 1, mem);
			}
			else {
				for (i = 0; i < path.size(); i++)
					Logger::error("Could not load cubemap, check these files: %s", path[i].c_str());
			}
		}
		else { // If the cubemap is catched we should not do anything, unless we have been told to upload it again
			if (forceLoad) {
				mem -= p_cubemap->mem; // Decrease the overall texture memory
				if (p_cubemap->load(path, flip)) {
					mem += p_cubemap->mem;
					Logger::info(LogLevel::med, "Cubemap %s[id:%d] force reload OK. Overall texture Memory : %.3fMb", path[0].c_str(), i, mem);
				}
				else {
					for (i = 0; i < path.size(); i++)
						Logger::error("Could not load cubemap, check these files: %s", path[i].c_str());
				}
			}
		}

		return p_cubemap;
	}

	int TextureManager::getOpenGLTextureID(int index)
	{
		if (index < texture.size())
			return texture[index]->m_textureID;
		return -1;
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
		mem = 0;
	}
}