// shader.cpp
// Spontz Demogroup

#include "../main.h"
#include "Shader.h"

#include <glm\gtc\type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>

Shader::Shader(const std::string & vertexShaderFilename,
               const std::string & fragmentShaderFilename,
               const std::string & geometryShaderFilename, 
               const std::string & tessellationControlShaderFilename, 
               const std::string & tessellationEvaluationShaderFilename) 
               : program_id(0), 
                 isLinked(false)
{
    const std::string shaderCodes[5] = { loadFile(vertexShaderFilename), 
                                         loadFile(fragmentShaderFilename), 
                                         loadFile(geometryShaderFilename),
                                         loadFile(tessellationControlShaderFilename),
                                         loadFile(tessellationEvaluationShaderFilename) };

    const std::string filenames[5] = { vertexShaderFilename, 
                                       fragmentShaderFilename, 
                                       geometryShaderFilename,
                                       tessellationControlShaderFilename,
                                       tessellationEvaluationShaderFilename };

    program_id = glCreateProgram();

    if (program_id == 0) {
        LOG->Error("Error while creating Shader program object");
        return;
    }

    for (int i = 0; i < sizeof(shaderCodes) / sizeof(std::string); ++i) {
        if (shaderCodes[i].empty()) {
            continue;
        }

        GLuint shaderType = 0;
        
        if (i == 0)
            shaderType = GL_VERTEX_SHADER;
        else
        if (i == 1)
            shaderType = GL_FRAGMENT_SHADER;
        else
        if (i == 2)
            shaderType = GL_GEOMETRY_SHADER;
        else
        if (i == 3)
            shaderType = GL_TESS_CONTROL_SHADER;
        else
        if (i == 4)
            shaderType = GL_TESS_EVALUATION_SHADER;

        if (shaderType == 0) {
            LOG->Error("Shader creation error: wrong shader type");
            continue;
        }

        GLuint shaderObject = glCreateShader(shaderType);

        if (shaderObject == 0) {
			LOG->Error("Error while creating shader: %s", filenames[i].c_str());
            continue;
        }

        const char *shaderCode[1] = { shaderCodes[i].c_str() };

        glShaderSource (shaderObject, 1, shaderCode, nullptr);
        glCompileShader(shaderObject);

        GLint result;
        glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

        if (result == GL_FALSE) {
			LOG->Error("%s compilation failed!", filenames[i].c_str());
            GLint logLen;
            glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &logLen);

            if (logLen > 0) {
                char * log = (char *)malloc(logLen);
                GLsizei written;
                glGetShaderInfoLog(shaderObject, logLen, &written, log);
				LOG->Error("Shader error log: %s", log);
                free(log);
            }

            continue;
        }

        glAttachShader(program_id, shaderObject);
        glDeleteShader(shaderObject);
    }

    link();
	// If all ok, copy the filenames to the shader
	this->fragmentShader_Filename = fragmentShaderFilename;
	this->vertexShader_Filename = vertexShaderFilename;
	this->geometryShader_Filename = geometryShaderFilename;
	this->tessellationControlShader_Filename = tessellationControlShaderFilename;
	this->tessellationEvaluationShader_Filename = tessellationEvaluationShaderFilename;
}

Shader::~Shader()
{
    if (program_id != 0) {
        glDeleteProgram(program_id);
        program_id = 0;
    }
}

bool Shader::link()
{
    glLinkProgram(program_id);

    GLint status;
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        LOG->Error("Failed to link shader program!");

        GLint logLen;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &logLen);

        if (logLen > 0) {
            char* log = (char*)malloc(logLen);
            GLsizei written;
            glGetProgramInfoLog(program_id, logLen, &written, log);
			LOG->Error("Shader Program log: %s", log);
            free(log);
        }
    }
    else {
        isLinked = true;
    }

    return isLinked;
}

void Shader::apply()
{
    if (program_id != 0 && isLinked) {
        glUseProgram(program_id);
    }
}

bool Shader::getUniformLocation(const std::string & uniform_name)
{
    GLint uniform_location = glGetUniformLocation(program_id, uniform_name.c_str());
    
    if (uniform_location != -1) {
        uniformsLocations[uniform_name] = uniform_location;
        return true;
    }
    else {
        LOG->Error("Error! Can't find uniform %s", uniform_name.c_str());
        return false;
    }
}

std::string Shader::loadFile(const std::string & filename)
{
	std::ifstream file(filename);
	if (!file) return std::string();

	file.ignore(std::numeric_limits<std::streamsize>::max());
	auto size = file.gcount();

	file.clear();
	file.seekg(0, std::ios_base::beg);

	std::stringstream sstr;
	sstr << file.rdbuf();
	file.close();

	return sstr.str();	
}

void Shader::setUniform1f(const std::string & uniformName, float value)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform1f(program_id, uniformsLocations[uniformName], value);
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniform1f(program_id, uniformsLocations[uniformName], value);
    }
}

void Shader::setUniform1i(const std::string & uniformName, int value)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform1i(program_id, uniformsLocations[uniformName], value);
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniform1i(program_id, uniformsLocations[uniformName], value);
    }
}

void Shader::setUniform1ui(const std::string & uniformName, unsigned int value)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform1ui(program_id, uniformsLocations.at(uniformName), value);
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniform1ui(program_id, uniformsLocations[uniformName], value);
    }
}

void Shader::setUniform1fv(const std::string & uniformName, GLsizei count, float * value)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform1fv(program_id, uniformsLocations[uniformName], count, value);
    }
    else
    {
        if (getUniformLocation(uniformName)) {
            glProgramUniform1fv(program_id, uniformsLocations[uniformName], count, value);
        }
    }
}

void Shader::setUniform1iv(const std::string & uniformName, GLsizei count, int * value)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform1iv(program_id, uniformsLocations[uniformName], count, value);
    }
    else
    {
        if (getUniformLocation(uniformName)) {
            glProgramUniform1iv(program_id, uniformsLocations[uniformName], count, value);
        }
    }
}

void Shader::setUniform2fv(const std::string & uniformName, const glm::vec2 & vector)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform2fv(program_id, uniformsLocations[uniformName], 1, glm::value_ptr(vector));
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniform2fv(program_id, uniformsLocations[uniformName], 1, glm::value_ptr(vector));
    }
}

void Shader::setUniform3fv(const std::string & uniformName, const glm::vec3 & vector)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform3fv(program_id, uniformsLocations[uniformName], 1, glm::value_ptr(vector));
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniform3fv(program_id, uniformsLocations[uniformName], 1, glm::value_ptr(vector));
    }
}

void Shader::setUniform4fv(const std::string & uniformName, const glm::vec4 & vector)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniform4fv(program_id, uniformsLocations[uniformName], 1, glm::value_ptr(vector));
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniform4fv(program_id, uniformsLocations[uniformName], 1, glm::value_ptr(vector));
    }
}

void Shader::setUniformMatrix3fv(const std::string & uniformName, const glm::mat3 & matrix)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniformMatrix3fv(program_id, uniformsLocations[uniformName], 1, GL_FALSE, glm::value_ptr(matrix));
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniformMatrix3fv(program_id, uniformsLocations[uniformName], 1, GL_FALSE, glm::value_ptr(matrix));
    }
}

void Shader::setUniformMatrix4fv(const std::string & uniformName, const glm::mat4 & matrix)
{
    if (uniformsLocations.count(uniformName)) {
        glProgramUniformMatrix4fv(program_id, uniformsLocations[uniformName], 1, GL_FALSE, glm::value_ptr(matrix));
    }
    else if (getUniformLocation(uniformName)) {
        glProgramUniformMatrix4fv(program_id, uniformsLocations[uniformName], 1, GL_FALSE, glm::value_ptr(matrix));
    }
}
