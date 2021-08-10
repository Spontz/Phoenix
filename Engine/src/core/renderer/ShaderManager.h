// ShaderManager.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Shader.h"

#include <memory>

namespace Phoenix {

	class ShaderManager final {

	public:
		ShaderManager();
		~ShaderManager();

	public:
		std::shared_ptr<Shader> addShader(
			std::string_view filepath,
			std::vector<std::string> const& feedbackVaryings = {}
		);
		void unbindShaders();
		void clear();

	public:
		std::vector<SP_Shader> shader; // Shader list
		// Force shader loading each time we add a shader (should be enabled when working on slave mode)
		bool forceLoad;
	};

}
