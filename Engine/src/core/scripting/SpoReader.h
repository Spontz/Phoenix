// SpoReader.h
// Spontz Demogroup

#pragma once

#include "main.h"

#include <variant>

namespace Phoenix {

	class SpoReader final {

	private:
		// defined section commands
		enum class SectionCommand {
			IDENTIFIER = 0,
			START,
			END,
			LAYER,
			ENABLED,
			BLEND,
			BLEND_EQUATION,
			PARAM,
			STRING,
			UNIFORM,
			SPLINE,
			MODIFIER,
			SHADER_BLOCK,
			FORMULA_BLOCK,
			INVALID
		};

		// defined shader commands
		enum class ShaderBlockCommand {
			STRING = 0,
			UNIFORM,
			INVALID
		};

		enum class SectionVar {
			TYPE_ERROR = 0,
			TYPE_BOOL,
			TYPE_INT,
			TYPE_FLOAT,
			TYPE_STRING
		};

		// Script command structure
		struct ScriptCommand final {
			std::string m_name;
			SectionVar m_type;
			void* m_pData;
		};

	public:
		SpoReader();
		~SpoReader();

	public:
		void readFromFile(std::string_view filePath);
		void readFromNetwork(std::string_view script);
		int	loadScriptData();

	private:
		bool			loadConfigScriptData();
		int				loadSectionScriptData();
		ShaderBlock*	loadShaderBlock(std::istringstream& f, int& lineNum);
		std::string		loadFormulaBlock(std::istringstream& f, int& lineNum);

		std::pair<std::string, std::string> splitIn2Lines(const std::string& line);
		std::vector<std::string> splitInMultipleLines(const std::string& line);
		std::string& removeDoubleSpaces(std::string& line);
		int32_t scriptCommandFound(const std::string& command);

	private:
		static const std::map<std::string, SectionCommand> spoSectionCommand;
		static const std::map<std::string, int32_t> spoBlendFunc;
		static const std::map<std::string, int32_t> spoBlendEquationFunc;

		static const std::map<std::string, ShaderBlockCommand> spoShaderBlockCommand;

	private:
		std::string m_filepath;
		std::string m_scriptData;

		// Script commands
		const std::vector<ScriptCommand> scriptCommand;
	};

}
