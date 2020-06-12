// shader.cpp
// Spontz Demogroup


/*
#include <glad/glad.h>
#include <glm/glm.hpp>
*/


#include "main.h"

#include "core/shader.h"
#include "core/utils/logger.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


Shader::Shader()
{
	ID = 0;
}

int Shader::load(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, std::vector<std::string> feedbackVaryings)
{
	// If we already have loaded this shader, we unload it first
	if (ID > 0) {
		glUseProgram(0);
		glDeleteProgram(ID);
		ID = 0;
	}

	vertexShader_Filename = vertexPath;
	fragmentShader_Filename = fragmentPath;
	geometryShader_Filename = geometryPath;
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (geometryPath != "") {
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e) {
		LOG->Error("Shader Error: Files have not been succesfully read, check paths: VERT: %s, FRAG: %s", vertexPath.c_str(), fragmentPath.c_str());
		return 0;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char * fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	if (checkCompileErrors(vertex, "VERTEX"))
		return 0;
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	if (checkCompileErrors(fragment, "FRAGMENT"))
		return 0;
	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (geometryPath != "") {
		const char * gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		if (checkCompileErrors(geometry, "GEOMETRY"))
			return 0;
	}
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != "")
		glAttachShader(ID, geometry);

	//Add the Transform Feedback Varyings
	
	if (feedbackVaryings.size() != 0) {
		// We convert the vector of stings to vector of chars
		std::vector<const char *> feedbackVaryings_cStr;
		feedbackVaryings_cStr.reserve(feedbackVaryings.size());
		for (int i = 0; i < feedbackVaryings.size(); ++i) {
			feedbackVaryings_cStr.push_back(feedbackVaryings[i].c_str());
		}
		glTransformFeedbackVaryings(ID, (GLsizei)feedbackVaryings_cStr.size(), &feedbackVaryings_cStr[0], GL_INTERLEAVED_ATTRIBS);
	}

	glLinkProgram(ID);
	if (checkCompileErrors(ID, "PROGRAM"))
		return 0;
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != "")
		glDeleteShader(geometry);
	return 1;
}

// activate the shader
// ------------------------------------------------------------------------
void Shader::use()
{
	glUseProgram(ID);
}
// Set utility uniform value functions
// ------------------------------------------------------------------------
void Shader::setValue(const char *name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name), (int)value);
}
void Shader::setValue(const char *name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name), value);
}
void Shader::setValue(const char *name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name), value);
}
void Shader::setValue(const char *name, const glm::vec2 &value) const
{
	glUniform2fv(glGetUniformLocation(ID, name), 1, &value[0]);
}
void Shader::setValue(const char *name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name), x, y);
}
void Shader::setValue(const char *name, const glm::vec3 &value) const
{
	glUniform3fv(glGetUniformLocation(ID, name), 1, &value[0]);
}
void Shader::setValue(const char *name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name), x, y, z);
}
void Shader::setValue(const char *name, const glm::vec4 &value) const
{
	glUniform4fv(glGetUniformLocation(ID, name), 1, &value[0]);
}
void Shader::setValue(const char *name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
}
void Shader::setValue(const char *name, const glm::mat2 &mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setValue(const char *name, const glm::mat3 &mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setValue(const char *name, const glm::mat4 &mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
}

// Get uniform location
// ------------------------------------------------------------------------
GLint Shader::getUniformLocation(const char *name) const
{
	GLint val;
	val = glGetUniformLocation(ID, name);
	if (val == -1)
		LOG->Info(LOG_MED, "Warning: Shader uniform variable '%s' not found in vs '%s' or in ps '%s'", name, vertexShader_Filename.c_str(), fragmentShader_Filename.c_str());
	return val;
}

// Check complie errors on shader
// ------------------------------------------------------------------------
bool Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLint size;
	GLchar *infoLog;
	bool errors = false;
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
			infoLog = new GLchar[size];
			glGetShaderInfoLog(shader, size, NULL, infoLog);
			std::string error_filename;
			if (type == "VERTEX")
				error_filename = this->vertexShader_Filename;
			if (type == "FRAGMENT")
				error_filename = this->fragmentShader_Filename;
			if (type == "GEOMETRY")
				error_filename = this->geometryShader_Filename;
			LOG->Error("Shader Compile (%s - %s) log: %s", type.c_str(), error_filename.c_str(), infoLog);
			delete[] infoLog;
			errors = true;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
			infoLog = (GLchar*)malloc(size);
			glGetProgramInfoLog(shader, size, NULL, infoLog);
			LOG->Error("Shader Linking: file %s, %s, log: %s", this->vertexShader_Filename.c_str(), this->fragmentShader_Filename.c_str(), infoLog);
			free(infoLog);
			errors = true;
		}
	}
	return errors;
}
