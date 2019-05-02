// shader.h
// Spontz Demogroup

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "main.h"
#include "core/utils/logger.h"



class Shader
{
public:
	unsigned int ID;
	string vertexShader_Filename;
	string fragmentShader_Filename;
	string geometryShader_Filename;
	// constructor generates the shader on the fly
	//Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string & geometryPath = "");
	// activate the shader
	void use();
	// utility uniform functions
	void setBool(const char *name, bool value) const;
	void setInt(const char *name, int value) const;
	void setFloat(const char *name, float value) const;
	void setVec2(const char *name, const glm::vec2 &value) const;
	void setVec2(const char *name, float x, float y) const;
	void setVec3(const char *name, const glm::vec3 &value) const;
	void setVec3(const char *name, float x, float y, float z) const;
	void setVec4(const char *name, const glm::vec4 &value) const;
	void setVec4(const char *name, float x, float y, float z, float w);
	void setMat2(const char *name, const glm::mat2 &mat) const;
	void setMat3(const char *name, const glm::mat3 &mat) const;
	void setMat4(const char *name, const glm::mat4 &mat) const;

	GLint getUniformLocation(int program_index, const char *varname) const;

private:
	// utility function for checking shader compilation/linking errors.
	void checkCompileErrors(GLuint shader, std::string type);
};
#endif