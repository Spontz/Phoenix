// ShaderManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ShaderManager.h"

namespace Phoenix {

	// Init vars
	ShaderManager::ShaderManager()
	{
		shader.clear();
		forceLoad = false;
	}

	ShaderManager::~ShaderManager()
	{
		clear();
	}

	// Adds a Texture into the queue, returns the ID of the texture added
	SP_Shader ShaderManager::addShader(
		std::string_view filepath,
		std::vector<std::string> const& feedbackVaryings
	)
	{
		bool loaded = false;
		int shad_id = -1;
		SP_Shader spShader;

		// check if shader is already loaded, then we just return the ID of our shader
		for (int i = 0; i < shader.size(); i++) {
			spShader = shader[i];
			if (spShader->getURI() == filepath) {
				shad_id = i;
				loaded = true; // Shader is already loaded
			}
		}

		SP_Shader spNewShader;

		if (!loaded) { // If the shader does not exist, we need to load it for the first time
			spNewShader = std::make_shared<Shader>();
			loaded = spNewShader->load(filepath, feedbackVaryings);
			if (loaded) {
				shader.push_back(spNewShader);
				shad_id = static_cast<int>(shader.size()) - 1;
			}
		}
		else {
			// If the shader is catched we should not do anything, unless we have been told to
			// upload it again
			if (forceLoad) {
				spNewShader = shader[shad_id];
				loaded = spNewShader->load(filepath, feedbackVaryings);
			}
			else {
				spNewShader = spShader;
				loaded = true;
			}
		}

		if (loaded) {
			Logger::info(
				LogLevel::med,
				"Shader loaded OK [id: %d, gl_id: %d] file: %s",
				shad_id,
				spNewShader->getId(),
				filepath.data()
			);
			return shader[shad_id];
		}

		else {
			Logger::error("Could not load shader: %s", filepath.data());
			return nullptr;
		}
	}

	void ShaderManager::unbindShaders()
	{
		glUseProgram(0);
	}

	void ShaderManager::clear()
	{
		shader.clear();
	}
}
