// texture_manager.h
// Spontz Demogroup

#pragma once

#include <string>
#include <vector>
#include "core/texture.h"
#include "core/cubemap.h"

class TextureManager {
public:
	std::vector<Texture*>	texture;	// Texture list
	std::vector<Cubemap*>	cubemap;	// Cubemap list
	float					mem;		// Texture memory in MegaBytes
	bool					forceLoad;	// Force texture loading each time we add a texture (should be enabled when working on slave mode)

	TextureManager();
	virtual ~TextureManager();
	Texture* addTexture(std::string path, bool flip = true, std::string type = "texture_diffuse");
	Cubemap* addCubemap(std::vector<std::string> path, bool flip);
	int	getOpenGLTextureID(int index);
	void initTextureStates();

private:


	
};