// TextureManager.h
// Spontz Demogroup

#pragma once

#include <string>
#include <vector>
#include "core/renderer/Texture.h"
#include "core/renderer/Cubemap.h"

namespace Phoenix {

	class TextureManager final {
	public:
		TextureManager();
		~TextureManager();

	public:
		SP_Texture	addTexture(std::string_view const& path);
		SP_Texture	addTexture(std::string_view const& path, Texture::Properties& texProperties);
		SP_Texture	addTextureFromMem(const unsigned char* data, int32_t len, Texture::Properties& texProperties);
		SP_Cubemap	addCubemap(std::vector<std::string> const& path, bool flip);
		int32_t		getOpenGLTextureID(int32_t index);
		void		initTextureStates();
		void		clear();

	public:
		std::vector<SP_Texture>	texture;	// Texture list
		std::vector<SP_Cubemap>	cubemap;	// Cubemap list
		float					m_mem;		// Texture memory in MegaBytes
		bool					m_forceLoad;// Force texture loading each time we add a texture (should be enabled when working on slave mode)
	};
}
