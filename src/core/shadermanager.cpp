// shadermanager.cpp
// Spontz Demogroup

#include "main.h"
#include "shadermanager.h"


// Init vars
ShaderManager::ShaderManager() {
	shader.clear();
}

// Adds a Texture into the queue, returns the ID of the texture added
int ShaderManager::addShader(string path_vert, string path_frag, string path_geom) {
	unsigned int i;
	int shad_id = -1;
	Shader *shad;
	path_vert = DEMO->dataFolder + path_vert;
	path_frag = DEMO->dataFolder + path_frag;
	if (path_geom!="")
		path_geom = DEMO->dataFolder + path_geom;

	// check if shader is already loaded, then we just return the ID of our shader
	for (i = 0; i < shader.size(); i++) {
		shad = shader[i];
		if ((shad->vertexShader_Filename.compare(path_vert) == 0)  &&
			(shad->fragmentShader_Filename.compare(path_frag) == 0) &&
			(shad->geometryShader_Filename.compare(path_geom) == 0)) {
			return i;
		}
	}
	// if we must load the shader...
	Shader *new_shad = new Shader();
	if (new_shad->load(path_vert, path_frag, path_geom)) {
		shader.push_back(new_shad);
		shad_id = (int)shader.size() - 1;
		LOG->Info(LOG_MED, "Shader [id: %d, gl_id: %d]: %s, %s  loaded OK", shad_id, new_shad->ID, path_vert.c_str(), path_frag.c_str());
	}
	else
		LOG->Error("Could not load shader: %s, %s", path_vert.c_str(), path_frag.c_str());
	
	return shad_id;
}

void ShaderManager::unbindShaders()
{
	glUseProgram(0);
}


