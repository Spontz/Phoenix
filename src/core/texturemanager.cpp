// texturemanager.cpp
// Spontz Demogroup

#include "main.h"
#include "texturemanager.h"


// Init vars
TextureManager::TextureManager() {
	texture.clear();
	cubemap.clear();
	mem = 0;
}

TextureManager::~TextureManager()
{
	texture.clear();
	cubemap.clear();
	mem = 0;
}

// Adds a Texture into the queue, returns the Number of the texture added
int TextureManager::addTexture(string path, bool flip, string type) {
	
	unsigned int i;
	int tex_num = -1;

	// check if texture is already loaded, then we just return the ID of our texture
	for (i = 0; i < texture.size(); i++) {
		if (texture[i]->filename.compare(path) == 0) {
			return i;
		}
	}
	// if we must load the texture...
	Texture *new_tex = new Texture();
	if (new_tex->load(path, flip)) {
		new_tex->type = type;
		texture.push_back(new_tex);
		mem += (float)(new_tex->width * new_tex->height * new_tex->components)/ 1048576.0f;		// increase the texture mem
		tex_num = (int)texture.size() - 1;
		LOG->Info(LOG_MED, "Texture %s [id: %d] loaded OK. Overall texture Memory: %.3fMb", path.c_str(), tex_num, mem);
	}
	else
		LOG->Error("Could not load texture: %s", path.c_str());
	
	return tex_num;
}

// Adds a Cubemap into the queue, returns the Number of the cubemap added
int TextureManager::addCubemap(vector<std::string> path, bool flip)
{
	unsigned int i;
	int cubemap_num = -1;

	// check if cubemap is already loaded, then we just return the ID of our texture
	bool already_loaded = true;
	for (i = 0; i < cubemap.size(); i++) {
		if (cubemap[i]->filename == path) // Check if all the paths are the same (so cubemap is already loaded)
		{
			return i;
		}
	}
	// if we must load the texture...
	Cubemap *new_cubemap = new Cubemap();
	if (new_cubemap->load(path, flip)) {
		cubemap.push_back(new_cubemap);
		cubemap_num = (int)cubemap.size() - 1;
		for (i=0; i<path.size(); i++)
			mem += (float)(new_cubemap->width[i] * new_cubemap->height[i] * 3) / 1048576.0f;		// increase the texture mem
		LOG->Info(LOG_MED, "Cubemap %s [id: %d] loaded OK. Overall texture Memory: %.3fMb", path[0].c_str(), cubemap_num, mem);
	}
	else {
		for (i=0; i<path.size(); i++)
			LOG->Error("Could not load cubemap, check these files: %s", path[i].c_str());
	}

	return cubemap_num;
}

int TextureManager::getOpenGLTextureID(int index)
{
	if (index < texture.size())
		return texture[index]->textureID;
	return -1;
}
