// shadermanager.cpp
// Spontz Demogroup

#include "main.h"
#include "shadermanager.h"


// Init vars
ShaderManager::ShaderManager() {
	
}

void ShaderManager::init() {
	LOG->Info(LOG_MED, "ShaderManager Inited. Clearing internal lists...");
	shader.clear();
}

// Adds a Texture into the queue, returns the ID of the texture added
int ShaderManager::addShader(string path_vert, string path_frag) {
	int i;
	int shad_id = -1;
	Shader *shad;

	// check if shader is already loaded, then we just return the ID of our shader
	for (i = 0; i < shader.size(); i++) {
		shad = shader[i];
		// TODO: Fix and add the geometry shader, tessellation shader, etc....
		if ((shad->vertexShader_Filename.compare(path_vert) == 0)  &&
			(shad->fragmentShader_Filename.compare(path_frag) == 0)) {
			return i;
		}
	}
	// if we must load the shader...
	Shader *new_shad = NULL;
	new_shad = new Shader(path_vert, path_frag);
	if (new_shad) {
		shader.push_back(new_shad);
		shad_id = (int)shader.size() - 1;
		LOG->Info(LOG_MED, "Shader %s, %s [id: %d] loaded OK", path_vert.c_str(), path_frag.c_str(), shad_id);
	}
	else
		LOG->Error("Could not load shader: %s, %s", path_vert.c_str(), path_frag.c_str());
	
	return shad_id;
}


