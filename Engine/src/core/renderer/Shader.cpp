// Shader.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Shader.h"
//#include "core/utils/LoggerDeclarations.h"

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
		};

		const auto it = s.find(type);
		return it == s.end() ? "UNKNOWN" : it->second.data();
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

	ShaderSources preprocessShaderSource(std::string_view shaderSource)
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
				Logger::error("Invalid shader type specified: {}", type);

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

	std::string readASCIIFile(std::string_view URI)
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
				Logger::error("Could not read file \"{}\".", URI);
			}
		}
		else {
			Logger::error("Could not open file \"{}\".", URI);
		}

		return result;
	}

	// Shader

	Shader::~Shader()
	{
		if (m_id != 0)
			glDeleteProgram(m_id);
	}

	// Loads a shader
	// Returns true is loaded OK
	// Returns false is failed loading shader
	bool Shader::load(std::string_view URI, const std::unordered_set<std::string>& feedbackVaryings)
	{
		// If we already have loaded this shader, we unload it first
		if (m_id > 0) {
			glUseProgram(0);
			glDeleteProgram(m_id);
			m_id = 0;
		}

		m_URI = URI;

		// 1. retrieve the vertex/fragment source code from filePath
		std::string source{ readASCIIFile(URI) };
		addLineDirective(source);

		return compile(preprocessShaderSource(source), feedbackVaryings);
	}

	// Activates the shader
	void Shader::use()
	{
		glUseProgram(m_id);
	}

	// Set utility uniform value functions
	void Shader::setValue(std::string_view name, GLint value) const
	{
		glUniform1i(glGetUniformLocation(m_id, name.data()), value);
	}

	void Shader::setValue(std::string_view name, GLfloat value) const
	{
		glUniform1f(glGetUniformLocation(m_id, name.data()), value);
	}

	void Shader::setValue(std::string_view name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(m_id, name.data()), 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y) const
	{
		glUniform2f(glGetUniformLocation(m_id, name.data()), x, y);
	}

	void Shader::setValue(std::string_view name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(m_id, name.data()), 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z) const
	{
		glUniform3f(glGetUniformLocation(m_id, name.data()), x, y, z);
	}

	void Shader::setValue(std::string_view name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(m_id, name.data()), 1, &value[0]);
	}

	void Shader::setValue(std::string_view name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
	{
		glUniform4f(glGetUniformLocation(m_id, name.data()), x, y, z, w);
	}

	void Shader::setValue(std::string_view name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(m_id, name.data()), 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::setValue(std::string_view name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(m_id, name.data()), 1, GL_FALSE, &mat[0][0]);
	}

	void Shader::setValue(std::string_view name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(m_id, name.data()), 1, GL_FALSE, &mat[0][0]);
	}

	std::string_view Shader::getURI() const
	{
		return m_URI;
	}

	uint32_t Shader::getId() const
	{
		return m_id;
	}

	GLint Shader::getUniformLocation(std::string_view name) const
	{
		const auto val = glGetUniformLocation(m_id, name.data());
		if (val == -1)
			Logger::info(
				LogLevel::med,
				"Warning: Shader uniform variable '{}' not found in shader '{}'",
				name,
				m_URI
			);
		return val;
	}

	// Compile shader
	// Returns true if succesfully compiled
	// Returns false if failed during loading
	bool Shader::compile(
		const ShaderSources& shaderSources,
		const std::unordered_set<std::string>& feedbackVaryings
	)
	{
		bool compiled = false;
		if (shaderSources.size() < 2) {
			return false;
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
