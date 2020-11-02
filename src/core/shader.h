// shader.h
// Spontz Demogroup

#ifndef SHADER_H
#define SHADER_H

#include "main.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader
{
public:
	unsigned int ID;
	std::string m_filepath;
	// constructor generates the shader on the fly
	Shader();
	virtual ~Shader();

	bool load(const std::string& filepath, std::vector<std::string> feedbackVaryings = { });

	// activate the shader
	void use();
	// utility uniform functions
	void setValue(const char *name, bool value) const;
	void setValue(const char *name, int value) const;
	void setValue(const char *name, float value) const;
	void setValue(const char *name, const glm::vec2 &value) const;
	void setValue(const char *name, float x, float y) const;
	void setValue(const char *name, const glm::vec3 &value) const;
	void setValue(const char *name, float x, float y, float z) const;
	void setValue(const char *name, const glm::vec4 &value) const;
	void setValue(const char *name, float x, float y, float z, float w);
	void setValue(const char *name, const glm::mat2 &mat) const;
	void setValue(const char *name, const glm::mat3 &mat) const;
	void setValue(const char *name, const glm::mat4 &mat) const;

	GLint getUniformLocation(const char *varname) const;

private:
	
	std::string ReadFile(const std::string& filepath);
	void addLinedirective(std::string& source);
	std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
	bool Compile(const std::unordered_map<GLenum, std::string>& shaderSources, std::vector<std::string> feedbackVaryings = { });

	GLenum Shader::GetShaderTypeFromString(const std::string& type);
	const std::string Shader::GetShaderStringFromType(const GLenum& type);
};
#endif