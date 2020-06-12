// shadermanager.cpp
// Spontz Demogroup

#include "main.h"
#include "shadermanager.h"


// Init vars
ShaderManager::ShaderManager() {
	shader.clear();
	forceLoad = false;
}

ShaderManager::~ShaderManager()
{
	shader.clear();
}

// Adds a Texture into the queue, returns the ID of the texture added
int ShaderManager::addShader(std::string path_vert, std::string path_frag, std::string path_geom, std::vector<std::string> feedbackVaryings) {
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
		else {
			new_shad = shad;
			loaded = true;
		}
			
	}

	if (loaded) {
		if (path_geom != "")
			LOG->Info(LOG_MED, "Shader loaded OK [id: %d, gl_id: %d] files: %s, %s, %s", shad_id, new_shad->ID, path_vert.c_str(), path_geom.c_str(), path_frag.c_str());
		else
			LOG->Info(LOG_MED, "Shader loaded OK [id: %d, gl_id: %d] files: %s, %s", shad_id, new_shad->ID, path_vert.c_str(), path_frag.c_str());
	}
		
	else
		LOG->Error("Could not load shader");


	return shad_id;
}

void ShaderManager::unbindShaders()
{
	glUseProgram(0);
}


