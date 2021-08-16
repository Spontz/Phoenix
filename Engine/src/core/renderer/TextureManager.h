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
		SP_Texture addTexture(std::string_view const& path, bool flip = true, std::string_view const& type = "texture_diffuse");
		SP_Texture addTextureFromMem(const unsigned char* data, int32_t len, bool flip, std::string_view const& type = "texture_diffuse");
		SP_Cubemap addCubemap(std::vector<std::string> const& path, bool flip); // TODO: Cubemap should be a SharedPointer, as it is in Texture class
		int32_t	getOpenGLTextureID(int32_t index);
		void initTextureStates();
		void clear();

	public:
		std::vector<SP_Texture>	texture;	// Texture list
		std::vector<SP_Cubemap>	cubemap;	// Cubemap list
		float					mem;		// Texture memory in MegaBytes
		bool					forceLoad;	// Force texture loading each time we add a texture (should be enabled when working on slave mode)
	};
}
