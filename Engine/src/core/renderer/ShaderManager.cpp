// ShaderManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/ShaderManager.h"

namespace Phoenix {

	ShaderManager::~ShaderManager()
	{
		clear();
	}

	// Adds a Texture into the queue, returns the ID of the texture added
	SP_Shader ShaderManager::addShader(
		std::string_view path,
		std::vector<std::string> const& feedbackVaryings
	)
	{
		bool loaded = false;
		int shad_id = -1;
		SP_Shader spShader;

		// check if shader is already loaded, then we just return the ID of our shader
		for (int i = 0; i < m_shader.size(); i++) {
			spShader = m_shader[i];
			if (spShader->getURI() == path) {
				shad_id = i;
				loaded = true; // Shader is already loaded
			}
		}

		SP_Shader spNewShader;

		if (!loaded) { // If the shader does not exist, we need to load it for the first time
			spNewShader = std::make_shared<Shader>();
			loaded = spNewShader->load(path, feedbackVaryings);
			if (loaded) {
				m_shader.push_back(spNewShader);
				shad_id = static_cast<int>(m_shader.size()) - 1;
			}
		}
		else {
			// If the shader is catched we should not do anything, unless we have been told to
			// upload it again
			if (m_forceLoad) {
				spNewShader = m_shader[shad_id];
				loaded = spNewShader->load(path, feedbackVaryings);
			}
			else {
				spNewShader = spShader;
				loaded = true;
			}
		}

		if (loaded) {
			Logger::info(
				LogLevel::med,
				"Shader loaded OK [id: {}, gl_id: {}] file: {}",
				shad_id,
				spNewShader->getId(),
				path
			);
			return m_shader[shad_id];
		}

		else {
			Logger::error("Could not load shader: {}", path);
			return nullptr;
		}
	}

	void ShaderManager::unbindShaders()
	{
		glUseProgram(0);
	}

	void ShaderManager::clear()
	{
		m_shader.clear();
	}
}
