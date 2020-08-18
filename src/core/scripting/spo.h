// spo.h
// Spontz Demogroup

#pragma once

#include "main.h"
//#include "core/demokernel.h"
//#include "core/drivers/gldriver.h"


namespace Phoenix {

	// defined section commands
	enum class SectionCommand_ : char const {
		IDENTIFIER = 0,
		START,
		END,
		LAYER,
		ENABLED,
		BLEND,
		BLEND_EQUATION,
		ALPHA,
		PARAM,
		STRING,
		UNIFORM,
		SPLINE,
		MODIFIER,
		INVALID
	};

	enum class SectionVar : char const {
		TYPE_ERROR = 0,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_STRING		
	};

	class SpoReader {
	public:
		SpoReader();
		virtual ~SpoReader();

		bool readAsciiFromFile(const std::string& filepath);
		bool readAsciiFromNetwork(std::string sScript);
		int	loadScriptData();


	private:
		std::string m_filepath;
		std::string m_scriptData;

		bool loadConfigScriptData();
		int loadSectionScriptData();

		std::pair<std::string, std::string> splitIn2Lines(const std::string& line);
		std::vector<std::string> splitInMultipleLines(const std::string& line);
		int scriptCommandFound(const std::string& command);
		
		const std::map<std::string, SectionCommand_> spoSectionCommand_ = {
			{"id",				SectionCommand_::IDENTIFIER},
			{"start",			SectionCommand_::START},
			{"end",				SectionCommand_::END},
			{"layer",			SectionCommand_::LAYER},
			{"enabled",			SectionCommand_::ENABLED},
			{"blend",			SectionCommand_::BLEND},
			{"blendequation",	SectionCommand_::BLEND_EQUATION},
			{"alpha",			SectionCommand_::ALPHA},
			{"param",			SectionCommand_::PARAM},
			{"string",			SectionCommand_::STRING},
			{"uniform",			SectionCommand_::UNIFORM},
			{"spline",			SectionCommand_::SPLINE},
			{"modify",			SectionCommand_::MODIFIER}
		};

		const std::map<std::string, int> spoBlendFunc = {
			{ "ZERO",						GL_ZERO							},
			{ "ONE",						GL_ONE							},
			{ "DST_COLOR",					GL_DST_COLOR					},
			{ "ONE_MINUS_DST_COLOR",		GL_ONE_MINUS_DST_COLOR			},
			{ "SRC_ALPHA",					GL_SRC_ALPHA					},
			{ "ONE_MINUS_SRC_ALPHA",		GL_ONE_MINUS_SRC_ALPHA			},
			{ "DST_ALPHA",					GL_DST_ALPHA					},
			{ "ONE_MINUS_DST_ALPHA",		GL_ONE_MINUS_DST_ALPHA			},
			{ "CONSTANT_COLOR",				GL_CONSTANT_COLOR				},
			{ "ONE_MINUS_CONSTANT_COLOR",	GL_ONE_MINUS_CONSTANT_COLOR		},
			{ "CONSTANT_ALPHA",				GL_CONSTANT_ALPHA				},
			{ "ONE_MINUS_CONSTANT_ALPHA",	GL_ONE_MINUS_CONSTANT_ALPHA		},
			{ "SRC_ALPHA_SATURATE",			GL_SRC_ALPHA_SATURATE			},
			{ "SRC_COLOR",					GL_SRC_COLOR					},
			{ "ONE_MINUS_SRC_COLOR",		GL_ONE_MINUS_SRC_COLOR			},
		};

		const std::map<std::string, int> spoBlendEquationFunc = {
			{ "ADD",				GL_FUNC_ADD				},
			{ "SUBTRACT",			GL_FUNC_SUBTRACT		},
			{ "REVERSE_SUBTRACT",	GL_FUNC_REVERSE_SUBTRACT}
		};

		const std::map<std::string, int> spoAlphaFunc = {
			{ "NEVER",		GL_NEVER	},
			{ "LESS",		GL_LESS		},
			{ "EQUAL",		GL_EQUAL	},
			{ "LEQUAL",		GL_LEQUAL	},
			{ "GREATER",	GL_GREATER	},
			{ "NOTEQUAL",	GL_NOTEQUAL	},
			{ "GEQUAL",		GL_GEQUAL	},
			{ "ALWAYS",		GL_ALWAYS	}
		};


		// Script command structure
		typedef struct {
			std::string cName;
			SectionVar vType;
			void* vAddr;
		} tScriptCommand;

		// Script commands
		const std::vector<tScriptCommand> scriptCommand;
	};

}