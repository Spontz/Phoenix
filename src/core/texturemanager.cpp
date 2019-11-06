// texturemanager.cpp
// Spontz Demogroup

#include "main.h"
#include "texturemanager.h"


// Init vars
TextureManager::TextureManager() {
	texture.clear();
	cubemap.clear();
	mem = 0;
	forceLoad = true;	// We force the texture to be reloaded, even if its already loaded, this should be only be true when the engine is in slave mode
#ifdef _DEBUG
	forceLoad = false;	// When debugging, forceLoad disabled, for faster loading!
#endif
}

TextureManager::~TextureManager()
{
	texture.clear();
	cubemap.clear();
	mem = 0;
}

int TextureManager::addTexture(string path, bool flip, string type) {
	unsigned int i;
	int tex_num = -1;

	// check if texture is already loaded, then we just retrieve the ID of our texture
	for (i = 0; i < texture.size(); i++) {
		if (texture[i]->filename.compare(path) == 0) {
			tex_num = i;
		}
	}

	if (tex_num == -1) { // If the texture has not been found, we need to load it for the first time
		Texture *new_tex = new Texture();
		if (new_tex->load(path, flip)) {
			new_tex->type = type;
			texture.push_back(new_tex);
			mem += new_tex->mem;
			tex_num = (int)texture.size() - 1;
			LOG->Info(LOG_MED, "Texture %s [id: %d] loaded OK. Overall texture Memory: %.3fMb", path.c_str(), tex_num, mem);
		}
		else
			LOG->Error("Could not load texture: %s", path.c_str());
	}
	else { // If the texture is catched we should not do anything, unless we have been told to upload it again
		if (forceLoad) {
			Texture *tex = texture[tex_num];
			mem -= tex->mem; // Decrease the overall texture memory
			if (tex->load(path, flip)) {
				tex->type = type;
				mem += tex->mem;
				LOG->Info(LOG_MED, "Texture %s [id: %d] force reload OK. Overall texture Memory: %.3fMb", path.c_str(), tex_num, mem);
			}
			else
				LOG->Error("Could not load texture: %s", path.c_str());
		}
	}

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
			cubemap_num = i;
		}
	}

	if (cubemap_num == -1) { // If the cubemap has not been found, we need to load it from the first time
		Cubemap *new_cubemap = new Cubemap();
		if (new_cubemap->load(path, flip)) {
			cubemap.push_back(new_cubemap);
			cubemap_num = (int)cubemap.size() - 1;
			mem += new_cubemap->mem;
			LOG->Info(LOG_MED, "Cubemap %s [id: %d] loaded OK. Overall texture Memory: %.3fMb", path[0].c_str(), cubemap_num, mem);
		}
		else {
			for (i = 0; i < path.size(); i++)
				LOG->Error("Could not load cubemap, check these files: %s", path[i].c_str());
		}
	}
	else { // If the cubemap is catched we should not do anything, unless we have been told to upload it again
		if (forceLoad) {
			Cubemap *cube = cubemap[cubemap_num];
			mem -= cube->mem; // Decrease the overall texture memory
			if (cube->load(path, flip)) {
				mem += cube->mem;
				LOG->Info(LOG_MED, "Cubemap %s[id:%d] force reload OK. Overall texture Memory : %.3fMb", path[0].c_str(), cubemap_num, mem);
			}
			else {
				for (i = 0; i < path.size(); i++)
					LOG->Error("Could not load cubemap, check these files: %s", path[i].c_str());
			}
		}
	}

	return cubemap_num;
}

int TextureManager::getOpenGLTextureID(int index)
{
	if (index < texture.size())
		return texture[index]->textureID;
	return -1;
}

void TextureManager::initTextureStates()
{
	// Bind no texture in texUnit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
