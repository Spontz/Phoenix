// Shader.h
// Spontz Demogroup

#pragma once
#include "main.h"

namespace Phoenix{

	class Shader;
	using SP_Shader = std::shared_ptr<Shader>;
	using WP_Shader = std::weak_ptr<Shader>;

	using ShaderSources = std::unordered_map<GLenum, std::string>;

	class Shader final {

	public:
		~Shader();

	public:
		bool load(std::string_view URI, std::vector<std::string> const& feedbackVaryings = {});
		void use(); // activate the shader

		// utility uniform functions
		void setValue(std::string_view id, GLint value) const;
		void setValue(std::string_view id, GLfloat value) const;
		void setValue(std::string_view id, const glm::vec2& value) const;
		void setValue(std::string_view id, GLfloat x, GLfloat y) const;
		void setValue(std::string_view id, const glm::vec3& value) const;
		void setValue(std::string_view id, GLfloat x, GLfloat y, GLfloat z) const;
		void setValue(std::string_view id, const glm::vec4& value) const;
		void setValue(std::string_view id, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		void setValue(std::string_view id, const glm::mat2& mat) const;
		void setValue(std::string_view id, const glm::mat3& mat) const;
		void setValue(std::string_view id, const glm::mat4& mat) const;

		GLint getUniformLocation(std::string_view id) const;

		std::string_view getURI() const { return m_URI; }
		uint32_t getId() const { return m_Id; }

	private:
		bool compile(const ShaderSources& shaderSources, std::vector<std::string> const& feedbackVaryings = {});
		// File management procedures
		std::string readASCIIFile(std::string_view URI);
		std::istream& safeGetline(std::istream& is, std::string& t);
		GLenum getShaderTypeFromString(std::string_view type);
		std::string_view getShaderStringFromType(const GLenum& type);
		void addLinedirective(std::string& source);
		ShaderSources preProcessShaderSource(std::string_view shaderSource);

	private:
		uint32_t m_Id = 0;
		std::string m_URI;
	};

}
