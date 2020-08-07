// spo.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/demokernel.h"
#include "core/drivers/gldriver.h"


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
		const std::vector<tScriptCommand> scriptCommand = {
			{"demo_name",				SectionVar::TYPE_STRING,		&DEMO->demoName			},
			{"debug",					SectionVar::TYPE_INT,			&DEMO->debug			},
			{"debugFontSize",			SectionVar::TYPE_FLOAT,			&DEMO->debug_fontSize	},
			{"loop",					SectionVar::TYPE_INT,			&DEMO->loop				},
			{"sound",					SectionVar::TYPE_INT,			&DEMO->sound			},
			{"demo_start",				SectionVar::TYPE_FLOAT,			&DEMO->startTime		},
			{"demo_end",				SectionVar::TYPE_FLOAT,			&DEMO->endTime			},
			{"slave",					SectionVar::TYPE_INT,			&DEMO->slaveMode		},
			{"log_detail",				SectionVar::TYPE_INT,			&DEMO->log_detail		},

			{"gl_fullscreen",			SectionVar::TYPE_INT,			&GLDRV->config.fullScreen		},
			{"gl_width",				SectionVar::TYPE_INT,			&GLDRV->config.framebuffer_width					},
			{"gl_height",				SectionVar::TYPE_INT,			&GLDRV->config.framebuffer_height				},
			{"gl_aspect",				SectionVar::TYPE_FLOAT,			&GLDRV->config.framebuffer_aspect_ratio	},
			{"gl_stencil",				SectionVar::TYPE_INT,			&GLDRV->config.stencil	},
			{"gl_vsync",				SectionVar::TYPE_INT,			&GLDRV->config.vsync	},

			{"fbo_0_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[0].ratio	},
			{"fbo_0_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[0].format	},
			{"fbo_0_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[0].numColorAttachments	},
			{"fbo_1_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[1].ratio	},
			{"fbo_1_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[1].format	},
			{"fbo_1_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[1].numColorAttachments	},
			{"fbo_2_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[2].ratio	},
			{"fbo_2_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[2].format	},
			{"fbo_2_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[2].numColorAttachments	},
			{"fbo_3_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[3].ratio	},
			{"fbo_3_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[3].format	},
			{"fbo_3_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[3].numColorAttachments	},
			{"fbo_4_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[4].ratio	},
			{"fbo_4_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[4].format	},
			{"fbo_4_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[4].numColorAttachments	},
			{"fbo_5_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[5].ratio	},
			{"fbo_5_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[5].format	},
			{"fbo_5_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[5].numColorAttachments	},
			{"fbo_6_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[6].ratio	},
			{"fbo_6_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[6].format	},
			{"fbo_6_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[6].numColorAttachments	},
			{"fbo_7_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[7].ratio	},
			{"fbo_7_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[7].format	},
			{"fbo_7_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[7].numColorAttachments	},
			{"fbo_8_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[8].ratio	},
			{"fbo_8_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[8].format	},
			{"fbo_8_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[8].numColorAttachments	},
			{"fbo_9_ratio",				SectionVar::TYPE_INT,			&GLDRV->fbo[9].ratio	},
			{"fbo_9_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[9].format	},
			{"fbo_9_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[9].numColorAttachments	},
			{"fbo_10_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[10].ratio	},
			{"fbo_10_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[10].format	},
			{"fbo_10_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[10].numColorAttachments	},
			{"fbo_11_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[11].ratio	},
			{"fbo_11_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[11].format	},
			{"fbo_11_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[11].numColorAttachments	},
			{"fbo_12_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[12].ratio	},
			{"fbo_12_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[12].format	},
			{"fbo_12_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[12].numColorAttachments	},
			{"fbo_13_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[13].ratio	},
			{"fbo_13_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[13].format	},
			{"fbo_13_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[13].numColorAttachments	},
			{"fbo_14_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[14].ratio	},
			{"fbo_14_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[14].format	},
			{"fbo_14_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[14].numColorAttachments	},
			{"fbo_15_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[15].ratio	},
			{"fbo_15_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[15].format	},
			{"fbo_15_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[15].numColorAttachments	},
			{"fbo_16_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[16].ratio	},
			{"fbo_16_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[16].format	},
			{"fbo_16_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[16].numColorAttachments	},
			{"fbo_17_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[17].ratio	},
			{"fbo_17_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[17].format	},
			{"fbo_17_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[17].numColorAttachments	},
			{"fbo_18_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[18].ratio	},
			{"fbo_18_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[18].format	},
			{"fbo_18_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[18].numColorAttachments	},
			{"fbo_19_ratio",			SectionVar::TYPE_INT,			&GLDRV->fbo[19].ratio	},
			{"fbo_19_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[19].format	},
			{"fbo_19_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[19].numColorAttachments	},

			{"fbo_20_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[20].width	},
			{"fbo_20_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[20].height	},
			{"fbo_20_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[20].format	},
			{"fbo_20_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[20].numColorAttachments	},
			{"fbo_21_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[21].width	},
			{"fbo_21_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[21].height	},
			{"fbo_21_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[21].format	},
			{"fbo_21_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[21].numColorAttachments	},
			{"fbo_22_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[22].width	},
			{"fbo_22_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[22].height	},
			{"fbo_22_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[22].format	},
			{"fbo_22_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[22].numColorAttachments	},
			{"fbo_23_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[23].width	},
			{"fbo_23_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[23].height	},
			{"fbo_23_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[23].format	},
			{"fbo_23_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[23].numColorAttachments },
			{"fbo_24_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[24].width	},
			{"fbo_24_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fbo[24].height	},
			{"fbo_24_format",			SectionVar::TYPE_STRING,		&GLDRV->fbo[24].format	},
			{"fbo_24_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fbo[24].numColorAttachments }
		};
	};

}