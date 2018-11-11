// texturemanager.cpp
// Spontz Demogroup

#include "main.h"
#include "texturemanager.h"


// Init vars
TextureManager::TextureManager() {
}

void TextureManager::init() {
	LOG->Info(LOG_MED, "TextureManager Inited. Clearing internal lists...");
	mem = 0;
	texture.clear();
}

// Adds a Texture into the queue, returns the ID of the texture added
int TextureManager::addTexture(string path) {
	int i;
	int tex_id = -1;

	// check if texture is already loaded, then we just return the ID of our texture
	for (i = 0; i < texture.size(); i++) {
		if (texture[i]->filename.compare(path) == 0) {
			return i;
		}
	}
	// if we must load the texture...
	Texture *new_tex = new Texture();
	if (new_tex->load(path)) {
		texture.push_back(new_tex);
		mem += (float)(new_tex->width * new_tex->height * new_tex->components)/ 1048576.0f;		// increase the texture mem
		tex_id = (int)texture.size() - 1;
		LOG->Info(LOG_MED, "Texture %s [id: %d] loaded OK. Overall texture Memory: %.3fMb", path.c_str(), tex_id, mem);
	}
	else
		LOG->Error("Could not load texture: %s", path.c_str());
	
	return tex_id;
}


