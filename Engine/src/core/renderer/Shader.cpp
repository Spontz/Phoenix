// Shader.cpp
// Spontz Demogroup


#include "main.h"

#include "core/renderer/Shader.h"
#include "core/utils/logger.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Phoenix {

	Shader::Shader()
	{
		ID = 0;
	}

	Shader::~Shader()
	{
		if (ID != 0) {
			glDeleteProgram(ID);
		}
	}

	// Load a shader
	// Return true is loaded OK
	// Return false is failed loading shader
	bool Shader::load(const std::string& filepath, std::vector<std::string> feedbackVaryings)
	{
		// If we already have loaded this shader, we unload it first
		if (ID > 0) {
			glUseProgram(0);
			glDeleteProgram(ID);
			ID = 0;
		}

		m_filepath = filepath;

		// 1. retrieve the vertex/fragment source code from filePath
		std::string source = ReadFile(filepath);
		addLinedirective(source);
		auto shaderSources = PreProcess(source);
		if (Compile(shaderSources, feedbackVaryings) == false)
			return false;
		else
			return true;
	}

	// activate the shader
	// ------------------------------------------------------------------------
	void Shader::use()
	{
		glUseProgram(ID);
	}
	// Set utility uniform value functions
	// ------------------------------------------------------------------------
	void Shader::setValue(const char* name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name), (int)value);
	}
	void Shader::setValue(const char* name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name), value);
	}
	void Shader::setValue(const char* name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name), value);
	}
	void Shader::setValue(const char* name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name), 1, &value[0]);
	}
	void Shader::setValue(const char* name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name), x, y);
	}
	void Shader::setValue(const char* name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name), 1, &value[0]);
	}
	void Shader::setValue(const char* name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name), x, y, z);
	}
	void Shader::setValue(const char* name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name), 1, &value[0]);
	}
	void Shader::setValue(const char* name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
	}
	void Shader::setValue(const char* name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
	}
	void Shader::setValue(const char* name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
	}
	void Shader::setValue(const char* name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
	}

	// Get uniform location
	// ------------------------------------------------------------------------
	GLint Shader::getUniformLocation(const char* name) const
	{
		GLint val;
		val = glGetUniformLocation(ID, name);
		if (val == -1)
			Logger::info(LogLevel::med, "Warning: Shader uniform variable '%s' not found in shader '%s'", name, m_filepath.c_str());
		return val;
	}


	std::string Shader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1) {
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
				in.close();
			}
			else {
				Logger::error("Could not read file %s", filepath.c_str());
			}
		}
		else {
			Logger::error("Could not open file %s", filepath.c_str());
		}

		return result;
	}

	std::istream& safeGetline(std::istream& is, std::string& t)
	{
		t.clear();

		// The characters in the stream are read one-by-one using a std::streambuf.
		// That is faster than reading them one-by-one using the std::istream.
		// Code that uses streambuf this way must be guarded by a sentry object.
		// The sentry object performs various tasks,
		// such as thread synchronization and updating the stream state.

		std::istream::sentry se(is, true);
		std::streambuf* sb = is.rdbuf();

		for (;;) {
			int c = sb->sbumpc();
			switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case std::streambuf::traits_type::eof():
				// Also handle the case when the last line has no line ending
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += (char)c;
			}
		}
	}

	void Shader::addLinedirective(std::string& source)
	{
		std::istringstream f(source);
		std::stringstream end_stream;
		std::string line;
		int lineNum = 1;
		while (!safeGetline(f, line).eof() && !f.fail()) {
			end_stream << line << std::endl;
			std::size_t found = line.find("#version"); // Right after the #version directive we will add the #line directive
			if (found != std::string::npos)
			{
				std::stringstream newline;
				newline << "#line " << lineNum << std::endl;
				end_stream << newline.str();
				lineNum++;
			}

			lineNum++;
		}
		source = end_stream.str();
	}

	std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		// Split shader by type
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			if (eol == std::string::npos)
				Logger::error("Shader PreProcess syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			if (0 == GetShaderTypeFromString(type))
				Logger::error("Invalid shader type specified: %s", type.c_str());

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			if (nextLinePos == std::string::npos)
				Logger::error("Shader PreProcess syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[GetShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}


	// Compile shader
	// return true if succesfully compiled
	// return false if failed during loading
	bool Shader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources, std::vector<std::string> feedbackVaryings)
	{
		bool compiled = false;
		if (shaderSources.size() < 2) {
			return false;
		}

		ID = glCreateProgram();
		std::vector<GLuint> glShaderIDs;
		glShaderIDs.reserve(shaderSources.size());
		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			if (kv.first == 0)
				return false;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				GLchar* infoLog = new GLchar[maxLength];
				glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

				glDeleteShader(shader);

				Logger::error("Shader Compile (%s - %s) log: %s", GetShaderStringFromType(type).c_str(), m_filepath.c_str(), infoLog);
				delete[] infoLog;
				return false;
			}

			glAttachShader(ID, shader);
			glShaderIDs.push_back(shader);
		}

		//Add the Transform Feedback Varyings
		if (feedbackVaryings.size() != 0) {
			// We convert the vector of stings to vector of chars
			std::vector<const char*> feedbackVaryings_cStr;
			feedbackVaryings_cStr.reserve(feedbackVaryings.size());
			for (int i = 0; i < feedbackVaryings.size(); ++i) {
				feedbackVaryings_cStr.push_back(feedbackVaryings[i].c_str());
			}
			glTransformFeedbackVaryings(ID, (GLsizei)feedbackVaryings_cStr.size(), &feedbackVaryings_cStr[0], GL_INTERLEAVED_ATTRIBS);
		}


		// Link our program
		glLinkProgram(ID);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(ID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			GLchar* infoLog = new GLchar[maxLength];
			glGetProgramInfoLog(ID, maxLength, &maxLength, infoLog);

			// We don't need the program anymore.
			glDeleteProgram(ID);

			for (auto shaderID : glShaderIDs)
				glDeleteShader(shaderID);


			Logger::error("Shader Linking: file %s, log: %s", this->m_filepath.c_str(), infoLog);
			delete[] infoLog;
			return false;
		}

		for (auto id : glShaderIDs)
		{
			glDetachShader(ID, id);
			glDeleteShader(id);
		}

		return true;
	}


	GLenum Shader::GetShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		if (type == "geometry")
			return GL_GEOMETRY_SHADER;

		return 0;
	}

	const std::string Shader::GetShaderStringFromType(const GLenum& type)
	{
		std::string s_type = "UNKNOWN";

		if (type == GL_VERTEX_SHADER)
			s_type = "Vertex";
		if (type == GL_FRAGMENT_SHADER)
			s_type = "Fragment";
		if (type == GL_GEOMETRY_SHADER)
			s_type = "Geometry";

		return s_type;
	}
}