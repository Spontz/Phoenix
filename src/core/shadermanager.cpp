// shadermanager.cpp
// Spontz Demogroup

#include "main.h"
#include "shadermanager.h"


// Init vars
ShaderManager::ShaderManager() {
	shader.clear();
	forceLoad = true;	// We force the shader to be reloaded, even if its already loaded, this should be only be true when the engine is in slave mode
}

ShaderManager::~ShaderManager()
{
	shader.clear();
}

// Adds a Texture into the queue, returns the ID of the texture added
int ShaderManager::addShader(string path_vert, string path_frag, string path_geom, vector<string> feedbackVaryings) {
	unsigned int i;
	int shad_id = -1;
	Shader *shad;

	// check if shader is already loaded, then we just return the ID of our shader
	for (i = 0; i < shader.size(); i++) {
		shad = shader[i];
		if ((shad->vertexShader_Filename.compare(path_vert) == 0)  &&
			(shad->fragmentShader_Filename.compare(path_frag) == 0) &&
			(shad->geometryShader_Filename.compare(path_geom) == 0)) {
			shad_id = i;
		}
	}

	Shader *new_shad;
	bool loaded = false;
	if (shad_id == -1) { // If the shader does not exist, we need to load it for the first time
		// if we must load the shader...
		new_shad = new Shader();
		loaded = new_shad->load(path_vert, path_frag, path_geom, feedbackVaryings);
		if (loaded) {
			shader.push_back(new_shad);
			shad_id = (int)shader.size() - 1;
		}
	}
	else {// If the shader is catched we should not do anything, unless we have been told to upload it again
		if (forceLoad) {
			new_shad = shader[shad_id];
			loaded = new_shad->load(path_vert, path_frag, path_geom, feedbackVaryings);
		}
		else
			loaded = true;
	}

	if(loaded)
		LOG->Info(LOG_MED, "Shader [id: %d, gl_id: %d]: %s, %s  loaded OK", shad_id, new_shad->ID, path_vert.c_str(), path_frag.c_str());
	else
		LOG->Error("Could not load shader: %s, %s", path_vert.c_str(), path_frag.c_str());


	return shad_id;
}

void ShaderManager::unbindShaders()
{
	glUseProgram(0);
}


