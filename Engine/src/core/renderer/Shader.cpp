// Shader.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Shader.h"
//#include "core/utils/LoggerDeclarations.h"

#include <cctype>

namespace Phoenix {

	// Helper functions

	std::istream& safeGetline(std::istream& is, std::string& t)
	{
		t.clear();

		// The characters in the stream are read one-by-one using a std::streambuf.
		// That is faster than reading them one-by-one using the std::istream.
		// Code that uses streambuf this way must be guarded by a sentry object.
		// The sentry object performs various tasks,
		// such as thread synchronization and updating the stream state.

		std::streambuf* sb = is.rdbuf();

		while(true) {
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

	GLenum getShaderTypeFromString(std::string_view type)
	{
		static const std::unordered_map<std::string_view, GLenum> s{
			{"vertex",   GL_VERTEX_SHADER   },
			{"fragment", GL_FRAGMENT_SHADER },
			{"geometry", GL_GEOMETRY_SHADER },
			{"compute",	 GL_COMPUTE_SHADER }
		};

		const auto it = s.find(type);
		return it == s.end() ? 0 : it->second;
	}

	std::string_view getShaderStringFromType(const GLenum& type)
	{
		static const std::unordered_map<GLenum, std::string_view> s{
			{ GL_VERTEX_SHADER,   "Vertex"   },
			{ GL_FRAGMENT_SHADER, "Fragment" },
			{ GL_GEOMETRY_SHADER, "Geometry" },
			{ GL_COMPUTE_SHADER, "Compute" }
		};

		const auto it = s.find(type);
		return it == s.end() ? "UNKNOWN" : it->second;
	}

	void addLineDirective(std::string& shaderSource)
	{
		std::istringstream f(shaderSource);
		std::stringstream end_stream;
		std::string line;
		int lineNum = 1;
		while (!safeGetline(f, line).eof() && !f.fail()) {
			end_stream << line << std::endl;
			// Right after the #version directive we will add the #line directive
			std::size_t found = line.find("#version");
			if (found != std::string::npos)
			{
				std::stringstream newline;
				newline << "#line " << lineNum << std::endl;
				end_stream << newline.str();
				lineNum++;
			}

			lineNum++;
		}
		shaderSource = end_stream.str();
	}

	bool Shader::preprocessShaderSource(
		std::string_view shaderSource,
		ShaderSources& shaderSources,
		std::string& error
	)
	{
		shaderSources.clear();
		bool inBlockComment = false;
		bool hasActiveStage = false;
		GLenum activeStage = 0;
		size_t lineStart = 0;
		uint32_t lineNumber = 1;

		auto updateBlockCommentState = [&inBlockComment](std::string_view line) {
			size_t position = 0;

			while (position < line.length()) {
				if (inBlockComment) {
					const size_t endComment = line.find("*/", position);
					if (endComment == std::string_view::npos)
						return;

					inBlockComment = false;
					position = endComment + 2;
					continue;
				}

				const size_t lineComment = line.find("//", position);
				const size_t blockComment = line.find("/*", position);
				if (lineComment != std::string_view::npos &&
					(blockComment == std::string_view::npos || lineComment < blockComment))
					return;

				if (blockComment == std::string_view::npos)
					return;

				inBlockComment = true;
				position = blockComment + 2;
			}
		};

		while (lineStart < shaderSource.length()) {
			const size_t lineEnd = shaderSource.find('\n', lineStart);
			const size_t nextLineStart = lineEnd == std::string_view::npos ? shaderSource.length() : lineEnd + 1;
			const size_t lineLength = (lineEnd == std::string_view::npos ? shaderSource.length() : lineEnd) - lineStart;
			const std::string_view line = shaderSource.substr(lineStart, lineLength);
			const bool isDirective = !inBlockComment && line.starts_with("#type") &&
				(line.length() == 5 || std::isspace(static_cast<unsigned char>(line[5])));

			if (isDirective) {
				std::istringstream declaration{ std::string(line.substr(5)) };
				std::string stageName;
				std::string trailingToken;
				declaration >> stageName >> trailingToken;

				if (stageName.empty() || !trailingToken.empty()) {
					error = "Shader PreProcess syntax error at line " + std::to_string(lineNumber);
					return false;
				}

				activeStage = getShaderTypeFromString(stageName);
				if (activeStage == 0) {
					error = "Invalid shader type specified at line " + std::to_string(lineNumber) + ": " + stageName;
					return false;
				}

				shaderSources[activeStage].clear();
				hasActiveStage = true;
			}
			else {
				if (hasActiveStage)
					shaderSources[activeStage].append(shaderSource.substr(lineStart, nextLineStart - lineStart));

				updateBlockCommentState(line);
			}

			lineStart = nextLineStart;
			++lineNumber;
		}

		return true;
	}

	Shader::~Shader()
	{
		if (m_id != 0)
			glDeleteProgram(m_id);
	}

	// Loads a shader
	// Returns true is loaded OK
	// Returns false is failed loading shader
	bool Shader::load(std::string_view URI, const std::vector<std::string>& feedbackVaryings)
	{
		// If we already have loaded this shader, we unload it first
		if (m_id > 0) {
			glUseProgram(0);
			glDeleteProgram(m_id);
			m_id = 0;
			m_UniformLocationCache.clear();
		}

		m_URI = URI;

		// retrieve the vertex/fragment source code from filePath
		std::string source{ Utils::readASCIIFile(URI) };
		addLineDirective(source);

		ShaderSources shaderSources;
		std::string parseError;
		if (!preprocessShaderSource(source, shaderSources, parseError)) {
			Logger::error("Shader '{}' could not be parsed: {}", URI, parseError);
			return false;
		}

		return compile(shaderSources, feedbackVaryings);
	}

	// Activates the shader
	void Shader::use()
	{
		glUseProgram(m_id);
	}

	bool Shader::isCompute() const
	{
		return m_programType == ShaderProgramType::Compute;
	}

	glm::uvec3 Shader::getComputeWorkGroupSize() const
	{
		if (!isCompute() || m_id == 0)
			return glm::uvec3(1);

		GLint workGroupSize[3] = { 1, 1, 1 };
		glGetProgramiv(m_id, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
		return glm::uvec3(static_cast<uint32_t>(workGroupSize[0]), static_cast<uint32_t>(workGroupSize[1]), static_cast<uint32_t>(workGroupSize[2]));
	}

	void Shader::dispatch(GLuint groupsX, GLuint groupsY, GLuint groupsZ)
	{
		if (!isCompute()) {
			Logger::error("Shader '{}' is not a compute shader", m_URI);
			return;
		}

		glDispatchCompute(groupsX, groupsY, groupsZ);
	}

	GLint Shader::getUniformLocation(std::string_view name) const
	{
		if (m_UniformLocationCache.find(name.data()) != m_UniformLocationCache.end())
			return m_UniformLocationCache[name.data()];

		GLint location = glGetUniformLocation(m_id, name.data());
		m_UniformLocationCache[name.data()] = location;

		return location;
	}

	// Set utility uniform value functions
	void Shader::setValue(std::string_view name, GLint value) const
	{
		GLint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform1i(loc, value);
	}

	void Shader::setValue(std::string_view name, GLuint value) const
	{
		GLint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform1ui(loc, value);
	}

	void Shader::setValue(std::string_view name, GLfloat value) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform1f(loc, value);
	}

	void Shader::setValue(std::string_view name, const glm::vec2& value) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform2fv(loc, 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform2f(loc, x, y);
	}

	void Shader::setValue(std::string_view name, const glm::vec3& value) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform3fv(loc, 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform3f(loc, x, y, z);
	}

	void Shader::setValue(std::string_view name, const glm::vec4& value) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform4fv(loc, 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniform4f(loc, x, y, z, w);
	}

	void Shader::setValue(std::string_view name, const glm::mat2& mat) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniformMatrix2fv(loc, 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::setValue(std::string_view name, const glm::mat3& mat) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniformMatrix3fv(loc, 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::setValue(std::string_view name, const glm::mat4& mat) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::setValue(std::string_view name, const glm::mat4& mat, GLsizei count) const
	{
		GLuint loc = getUniformLocation(name);
		if (loc != -1)
			glUniformMatrix4fv(loc, count, GL_FALSE, &mat[0][0]);
	}

	std::string_view Shader::getURI() const
	{
		return m_URI;
	}

	uint32_t Shader::getId() const
	{
		return m_id;
	}

	// Compile shader
	// Returns true if succesfully compiled
	// Returns false if failed during loading
	bool Shader::compile(
		const ShaderSources& shaderSources,
		const std::vector<std::string>& feedbackVaryings
	)
	{
		// Detect if the shader is a traditional shader (vertex + fragment + geometry) or a compute shader
		const bool hasVertex =	 shaderSources.contains(GL_VERTEX_SHADER);
		const bool hasFragment = shaderSources.contains(GL_FRAGMENT_SHADER);
		const bool hasGeometry = shaderSources.contains(GL_GEOMETRY_SHADER);
		const bool hasCompute =  shaderSources.contains(GL_COMPUTE_SHADER);

		const bool isComputeProgram = hasCompute;

		if (isComputeProgram) {
			if (hasVertex || hasFragment ||	hasGeometry) {
				Logger::error("Invalid compute shader '{}': compute shaders cannot contain vertex, fragment or geometry stages", m_URI);
				return false;
			}
			if (!feedbackVaryings.empty()) {
				Logger::error("Invalid compute shader '{}': transform feedback varyings are not valid for compute shaders", m_URI);
				return false;
			}
			m_programType = ShaderProgramType::Compute;
		}
		else {
			if (!hasVertex || !hasFragment) {
				Logger::error("Invalid traditional shader '{}': vertex and fragment stages are required", m_URI);
				return false;
			}
			m_programType = ShaderProgramType::Traditional;
		}

		m_id = glCreateProgram();
		std::vector<GLuint> glShaderIDs;
		glShaderIDs.reserve(shaderSources.size());

		for (auto const& [type, source] : shaderSources) {
			if (type == 0)
				return false;

			const GLuint shader = glCreateShader(type);
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE) {
				GLint maxLength;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				GLchar* infoLog = new GLchar[maxLength];
				glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

				glDeleteShader(shader);

				Logger::error(
					"Shader Compile ({} - {}) log: {}",
					getShaderStringFromType(type),
					m_URI,
					infoLog
				);
				delete[] infoLog;
				return false;
			}

			glAttachShader(m_id, shader);
			glShaderIDs.emplace_back(shader);
		}

		// Add the Transform Feedback Varyings
		if (!feedbackVaryings.empty()) {
			// Convert the vector of strings to vector of const char*'s
			std::vector<const char*> feedbackVaryings_cStr;
			feedbackVaryings_cStr.reserve(feedbackVaryings.size());
			for (auto const& i : feedbackVaryings)
				feedbackVaryings_cStr.emplace_back(i.c_str());

			glTransformFeedbackVaryings(
				m_id,
				static_cast<GLsizei>(feedbackVaryings_cStr.size()),
				feedbackVaryings_cStr.data(),
				GL_INTERLEAVED_ATTRIBS
			);
		}


		// Link our program
		glLinkProgram(m_id);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked;
		glGetProgramiv(m_id, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength;
			glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			GLchar* infoLog = new GLchar[maxLength];
			glGetProgramInfoLog(m_id, maxLength, &maxLength, infoLog);

			// We don't need the program anymore.
			glDeleteProgram(m_id);

			for (auto shaderID : glShaderIDs)
				glDeleteShader(shaderID);


			Logger::error("Shader Linking: file {}, log: {}", m_URI, infoLog);
			delete[] infoLog;
			return false;
		}

		for (auto id : glShaderIDs) {
			glDetachShader(m_id, id);
			glDeleteShader(id);
		}

		return true;
	}

}
