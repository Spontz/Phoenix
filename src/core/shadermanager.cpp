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
Shader* ShaderManager::addShader(std::string filepath, std::vector<std::string> feedbackVaryings) {
	unsigned int i;
	bool loaded = false;
	int shad_id = -1;
	Shader *shad;

	// check if shader is already loaded, then we just return the ID of our shader
	for (i = 0; i < shader.size(); i++) {
		shad = shader[i];
		if (shad->m_filepath.compare(filepath) == 0) {
			shad_id = i;
			loaded = true; // Shader is already loaded
		}
	}

	Shader *new_shad;

	if (!loaded) { // If the shader does not exist, we need to load it for the first time
		new_shad = new Shader();
		loaded = new_shad->load(filepath, feedbackVaryings);
		if (loaded) {
			shader.push_back(new_shad);
			shad_id = (int)shader.size() - 1;
		}
	}
	else {// If the shader is catched we should not do anything, unless we have been told to upload it again
		if (forceLoad) {
			new_shad = shader[shad_id];
			loaded = new_shad->load(filepath, feedbackVaryings);
		}
		else {
			new_shad = shad;
			loaded = true;
		}
			
	}

	if (loaded) {
		LOG->Info(LogLevel::MED, "Shader loaded OK [id: %d, gl_id: %d] file: %s", shad_id, new_shad->ID, filepath.c_str());
		return shader[shad_id];
	}
		
	else {
		LOG->Error("Could not load shader: %s", filepath.c_str());
		return nullptr;
	}
}

void ShaderManager::unbindShaders()
{
	glUseProgram(0);
}