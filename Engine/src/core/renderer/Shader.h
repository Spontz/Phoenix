// Shader.h
// Spontz Demogroup

#pragma once
#include "main.h"

#include <unordered_set>
#include <unordered_map>

namespace Phoenix{

	class Shader;
	using SP_Shader = std::shared_ptr<Shader>;
	using WP_Shader = std::weak_ptr<Shader>;

	using ShaderSources = std::unordered_map<GLenum, std::string>;

	class Shader final {

	public:
		~Shader();

	public:
		bool load(std::string_view path, std::vector<std::string> const& feedbackVaryings);
		void use(); // activate the shader

		// Get uniform location
		GLint getUniformLocation(std::string_view name) const;

		// set uniform scalar
		void setValue(std::string_view name, GLint value) const;
		void setValue(std::string_view name, GLuint value) const;
		void setValue(std::string_view name, GLfloat value) const;

		// set uniform vector from components
		void setValue(std::string_view name, GLfloat x, GLfloat y) const;
		void setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z) const;
		void setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const;

		// set uniform vector from glm vector
		void setValue(std::string_view name, const glm::vec2& value) const;
		void setValue(std::string_view name, const glm::vec3& value) const;
		void setValue(std::string_view name, const glm::vec4& value) const;

		// set uniform matrix from glm matrix
		void setValue(std::string_view name, const glm::mat2& mat) const;
		void setValue(std::string_view name, const glm::mat3& mat) const;
		void setValue(std::string_view name, const glm::mat4& mat) const;
		void setValue(std::string_view name, const glm::mat4& mat, GLsizei count) const;

		std::string_view getURI() const;
		uint32_t getId() const;

	private:
		bool compile(
			const ShaderSources& shaderSources,
			std::vector<std::string> const& feedbackVaryings
		);

	private:
		uint32_t m_id = 0;
		std::string m_URI;
		mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;
	};

}
