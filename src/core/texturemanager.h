// texturenmanager.h
// Spontz Demogroup

#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include <string>
#include <vector>
#include "core/texture.h"
#include "core/cubemap.h"

using namespace std;

class TextureManager {
public:
	std::vector<Texture*>	texture;	// Texture list
	std::vector<Cubemap*>	cubemap;	// Cubemap list
	float					mem;		// Texture memory in MegaBytes
	bool					forceLoad;	// Force texture loading each time we add a texture (should be enabled when working on slave mode)

	TextureManager();
	virtual ~TextureManager();
	int addTexture(string path, bool flip = true, string type = "texture_diffuse");
	int addCubemap(vector<std::string> path, bool flip);
	int	getOpenGLTextureID(int index);
	void initTextureStates();

private:


	
};

#endif