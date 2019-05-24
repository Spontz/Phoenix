// texturenmanager.h
// Spontz Demogroup

#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include <string>
#include <vector>
#include "core/texture.h"

using namespace std;

class TextureManager {
public:
	std::vector<Texture*> texture;	// Texture list
	float mem;						// Texture memory in MegaBytes

	TextureManager();
	int addTexture(string path, bool flip = true);
	int	getOpenGLTextureID(int index);

private:


	
};

#endif