// Shader.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Shader.h"
#include "core/utils/logger.h"

namespace Phoenix {

	std::istream& Shader::safeGetline(std::istream& is, std::string& t)
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

	GLenum Shader::getShaderTypeFromString(std::string_view type)
	{
		static const std::unordered_map<std::string_view, GLenum> s{
			{"vertex",   GL_VERTEX_SHADER   },
			{"fragment", GL_FRAGMENT_SHADER },
			{"geometry", GL_GEOMETRY_SHADER },
		};

		const auto it = s.find(type);
		return it == s.end() ? 0 : it->second;
	}

	std::string_view Shader::getShaderStringFromType(const GLenum& type)
	{
		static const std::unordered_map<GLenum, std::string_view> s{
			{ GL_VERTEX_SHADER,   "Vertex"   },
			{ GL_FRAGMENT_SHADER, "Fragment" },
			{ GL_GEOMETRY_SHADER, "Geometry" },
		};

		const auto it = s.find(type);
		return it == s.end() ? "UNKNOWN" : it->second.data();
	}

	void Shader::addLinedirective(std::string& source)
	{
		std::istringstream f(source);
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
		source = end_stream.str();
	}

	ShaderSources Shader::preProcessShaderSource(std::string_view shaderSource)
	{
		ShaderSources shaderSources;

		// Split shader by type
		static constexpr std::string_view typeToken("#type");

		size_t pos = shaderSource.find(typeToken, 0); // Start of shader type declaration line

		while (pos != std::string::npos) {
			// End of shader type declaration line
			const auto eol = shaderSource.find_first_of("\r\n", pos);
			if (eol == std::string::npos)
				Logger::error("Shader PreProcess syntax error");

			// Start of shader type name (after "#type " keyword)
			const auto begin = pos + typeToken.length() + 1;
			const auto type = shaderSource.substr(begin, eol - begin);
			if (getShaderTypeFromString(type) == 0)
				Logger::error("Invalid shader type specified: %s", type.data());

			// Start of shader code after shader type declaration line
			const auto nextLinePos = shaderSource.find_first_not_of("\r\n", eol);
			if (nextLinePos == std::string::npos)
				Logger::error("Shader PreProcess syntax error");

			// Start of next shader type declaration line
			pos = shaderSource.find(typeToken, nextLinePos);
			shaderSources[getShaderTypeFromString(type)] = (pos == std::string::npos) ?
				shaderSource.substr(nextLinePos)
				:
				shaderSource.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	std::string Shader::readASCIIFile(std::string_view URI)
	{
		std::string result;
		std::ifstream in(URI.data(), std::ios::in | std::ios::binary);
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
				Logger::error("Could not read file \"%s\".", URI.data());
			}
		}
		else {
			Logger::error("Could not open file \"%s\".", URI.data());
		}

		return result;
	}

	Shader::~Shader()
	{
		if (m_Id != 0)
			glDeleteProgram(m_Id);
	}

	// Loads a shader
	// Returns true is loaded OK
	// Returns false is failed loading shader
	bool Shader::load(std::string_view URI, std::vector<std::string> const& feedbackVaryings)
	{
		// If we already have loaded this shader, we unload it first
		if (m_Id > 0) {
			glUseProgram(0);
			glDeleteProgram(m_Id);
			m_Id = 0;
		}

		m_URI = URI;

		// 1. retrieve the vertex/fragment source code from filePath
		std::string source{ readASCIIFile(URI) };
		addLinedirective(source);
		auto shaderSources = preProcessShaderSource(source);
		if (compile(shaderSources, feedbackVaryings) == false)
			return false;
		else
			return true;
	}

	// Activates the shader
	void Shader::use()
	{
		glUseProgram(m_Id);
	}

	// Set utility uniform value functions
	void Shader::setValue(std::string_view name, GLint value) const
	{
		glUniform1i(glGetUniformLocation(m_Id, name.data()), value);
	}

	void Shader::setValue(std::string_view name, GLfloat value) const
	{
		glUniform1f(glGetUniformLocation(m_Id, name.data()), value);
	}

	void Shader::setValue(std::string_view name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(m_Id, name.data()), 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y) const
	{
		glUniform2f(glGetUniformLocation(m_Id, name.data()), x, y);
	}

	void Shader::setValue(std::string_view name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(m_Id, name.data()), 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z) const
	{
		glUniform3f(glGetUniformLocation(m_Id, name.data()), x, y, z);
	}

	void Shader::setValue(std::string_view name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(m_Id, name.data()), 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
	{
		glUniform4f(glGetUniformLocation(m_Id, name.data()), x, y, z, w);
	}

	void Shader::setValue(std::string_view name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(m_Id, name.data()), 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::setValue(std::string_view name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(m_Id, name.data()), 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::setValue(std::string_view name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(m_Id, name.data()), 1, GL_FALSE, &mat[0][0]);
	}

	GLint Shader::getUniformLocation(std::string_view name) const
	{
		const auto val = glGetUniformLocation(m_Id, name.data());
		if (val == -1)
			Logger::info(
				LogLevel::med,
				"Warning: Shader uniform variable '%s' not found in shader '%s'",
				name,
				m_URI.c_str()
			);
		return val;
	}

	// Compile shader
	// Returns true if succesfully compiled
	// Returns false if failed during loading
	bool Shader::compile(
		const ShaderSources& shaderSources,
		std::vector<std::string> const& feedbackVaryings
	)
	{
		bool compiled = false;
		if (shaderSources.size() < 2) {
			return false;
		}

		m_Id = glCreateProgram();
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
					"Shader Compile (%s - %s) log: %s",
					getShaderStringFromType(type).data(),
					m_URI.c_str(),
					infoLog
				);
				delete[] infoLog;
				return false;
			}

			glAttachShader(m_Id, shader);
			glShaderIDs.push_back(shader);
		}

		// Add the Transform Feedback Varyings
		if (!feedbackVaryings.empty()) {
			// Convert the vector of strings to vector of const char*'s
			std::vector<const char*> feedbackVaryings_cStr;
			feedbackVaryings_cStr.reserve(feedbackVaryings.size());
			for (auto const& i : feedbackVaryings)
				feedbackVaryings_cStr.emplace_back(i.c_str());

			glTransformFeedbackVaryings(
				m_Id,
				static_cast<GLsizei>(feedbackVaryings_cStr.size()),
				feedbackVaryings_cStr.data(),
				GL_INTERLEAVED_ATTRIBS
			);
		}


		// Link our program
		glLinkProgram(m_Id);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked;
		glGetProgramiv(m_Id, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength;
			glGetProgramiv(m_Id, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			GLchar* infoLog = new GLchar[maxLength];
			glGetProgramInfoLog(m_Id, maxLength, &maxLength, infoLog);

			// We don't need the program anymore.
			glDeleteProgram(m_Id);

			for (auto shaderID : glShaderIDs)
				glDeleteShader(shaderID);


			Logger::error("Shader Linking: file %s, log: %s", this->m_URI.c_str(), infoLog);
			delete[] infoLog;
			return false;
		}

		for (auto id : glShaderIDs) {
			glDetachShader(m_Id, id);
			glDeleteShader(id);
		}

		return true;
	}

}
