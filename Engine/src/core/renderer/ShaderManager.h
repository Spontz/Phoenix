// ShaderManager.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Shader.h"

namespace Phoenix {

	class ShaderManager final
	{
		friend class DemoKernel;
		friend class ImGuiLayer;

	public:
		~ShaderManager();

	public:
		SP_Shader addShader(
			std::string_view path,
			const std::vector<std::string>& feedbackVaryings = {}
		);
		void unbindShaders();
		void clear();

	private:
		std::vector<SP_Shader> m_shader; // Shader list
		// Force shader loading each time we add a shader (should be enabled when working on slave mode)
		bool m_forceLoad = false;
	};

}
