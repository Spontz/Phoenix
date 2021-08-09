// TextureManager.h
// Spontz Demogroup

#pragma once

#include <string>
#include <vector>
#include "core/renderer/Texture.h"
#include "core/renderer/Cubemap.h"

namespace Phoenix {

	class TextureManager {
	public:
		std::vector<Texture*>	texture;	// Texture list
		std::vector<Cubemap*>	cubemap;	// Cubemap list
		float					mem;		// Texture memory in MegaBytes
		bool					forceLoad;	// Force texture loading each time we add a texture (should be enabled when working on slave mode)

		TextureManager();
		virtual ~TextureManager();
		Texture* addTexture(std::string_view const& path, bool flip = true, std::string_view const& type = "texture_diffuse");
		Texture* addTextureFromMem(const unsigned char* data, int len, bool flip, std::string_view const& type = "texture_diffuse");
		Cubemap* addCubemap(std::vector<std::string> const& path, bool flip);
		int	getOpenGLTextureID(int index);
		void initTextureStates();
		void clear();
	};
}
