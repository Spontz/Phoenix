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
		SP_Texture	addTexture(std::vector<std::string> path, Texture::Properties& texProperties);
		SP_Texture	addTextureFromMem(const unsigned char* data, int32_t len, Texture::Properties& texProperties);
		SP_Cubemap	addCubemap(std::vector<std::string> const& path, bool flip);
		int32_t		getOpenGLTextureID(int32_t index);
		// Runtime 3D texture pool (sampler3D) for use in compute shaders, etc.
		bool		initRuntime3D(int count, int width, int height, int depth, std::string_view format, bool useLinearFilter);
		SP_Texture	getRuntime3D(int index) const;
		void		initTextureStates();
		void		clear();

	public:
		std::vector<SP_Texture>	texture;	// Texture list
		std::vector<SP_Cubemap>	cubemap;	// Cubemap list
		std::vector<SP_Texture>	runtime3D;	// Runtime 3D texture list (sampler3D)
		float					m_mem;		// Texture memory in MegaBytes
		bool					m_forceLoad;// Force texture loading each time we add a texture (should be enabled when working on slave mode)
	};
}
