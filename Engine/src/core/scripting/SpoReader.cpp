// SpoReader.cpp
// Spontz Demogroup

#include "core/scripting/SpoReader.h"

namespace Phoenix {

	const std::map<std::string, SpoReader::SectionCommand> SpoReader::spoSectionCommand = {
		{"id",				SectionCommand::IDENTIFIER},
		{"start",			SectionCommand::START},
		{"end",				SectionCommand::END},
		{"layer",			SectionCommand::LAYER},
		{"enabled",			SectionCommand::ENABLED},
		{"blend",			SectionCommand::BLEND},
		{"blendequation",	SectionCommand::BLEND_EQUATION},
		{"param",			SectionCommand::PARAM},
		{"string",			SectionCommand::STRING},
		{"uniform",			SectionCommand::UNIFORM},
		{"spline",			SectionCommand::SPLINE},
		{"modify",			SectionCommand::MODIFIER}
	};

	const std::map<std::string, int> SpoReader::spoBlendFunc = {
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

	const std::map<std::string, int> SpoReader::spoBlendEquationFunc = {
		{ "ADD",				GL_FUNC_ADD				},
		{ "SUBTRACT",			GL_FUNC_SUBTRACT		},
		{ "REVERSE_SUBTRACT",	GL_FUNC_REVERSE_SUBTRACT}
	};


	SpoReader::SpoReader()
		:
		m_filepath(""),
		m_scriptData(""),
		scriptCommand({
				{"demo_name",				SectionVar::TYPE_STRING,		&DEMO->m_demoName		},
				{"debug",					SectionVar::TYPE_BOOL,			&DEMO->m_debug			},
				{"debugFontSize",			SectionVar::TYPE_FLOAT,			&DEMO->m_debugFontSize	},
				{"loop",					SectionVar::TYPE_BOOL,			&DEMO->m_loop			},
				{"sound",					SectionVar::TYPE_BOOL,			&DEMO->m_sound			},
				{"demo_start",				SectionVar::TYPE_FLOAT,			&DEMO->m_demoStartTime	},
				{"demo_end",				SectionVar::TYPE_FLOAT,			&DEMO->m_demoEndTime	},
				{"slave",					SectionVar::TYPE_BOOL,			&DEMO->m_slaveMode		},
				{"log_detail",				SectionVar::TYPE_INT,			&DEMO->m_logLevel		},

				{"gl_fullscreen",			SectionVar::TYPE_BOOL,			&GLDRV->config.fullScreen				},
				{"gl_width",				SectionVar::TYPE_INT,			&GLDRV->config.framebuffer_width		},
				{"gl_height",				SectionVar::TYPE_INT,			&GLDRV->config.framebuffer_height		},
				{"gl_aspect",				SectionVar::TYPE_FLOAT,			&GLDRV->config.framebuffer_aspect_ratio	},
				{"gl_stencil",				SectionVar::TYPE_INT,			&GLDRV->config.stencil					},
				{"gl_vsync",				SectionVar::TYPE_INT,			&GLDRV->config.vsync					},
				{"gl_multisample",			SectionVar::TYPE_INT,			&GLDRV->config.multisampling			},

				{"fbo_0_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[0].ratio	},
				{"fbo_0_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[0].format	},
				{"fbo_0_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[0].numColorAttachments	},
				{"fbo_1_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[1].ratio	},
				{"fbo_1_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[1].format	},
				{"fbo_1_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[1].numColorAttachments	},
				{"fbo_2_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[2].ratio	},
				{"fbo_2_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[2].format	},
				{"fbo_2_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[2].numColorAttachments	},
				{"fbo_3_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[3].ratio	},
				{"fbo_3_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[3].format	},
				{"fbo_3_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[3].numColorAttachments	},
				{"fbo_4_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[4].ratio	},
				{"fbo_4_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[4].format	},
				{"fbo_4_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[4].numColorAttachments	},
				{"fbo_5_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[5].ratio	},
				{"fbo_5_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[5].format	},
				{"fbo_5_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[5].numColorAttachments	},
				{"fbo_6_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[6].ratio	},
				{"fbo_6_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[6].format	},
				{"fbo_6_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[6].numColorAttachments	},
				{"fbo_7_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[7].ratio	},
				{"fbo_7_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[7].format	},
				{"fbo_7_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[7].numColorAttachments	},
				{"fbo_8_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[8].ratio	},
				{"fbo_8_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[8].format	},
				{"fbo_8_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[8].numColorAttachments	},
				{"fbo_9_ratio",				SectionVar::TYPE_INT,			&GLDRV->fboConfig[9].ratio	},
				{"fbo_9_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[9].format	},
				{"fbo_9_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[9].numColorAttachments	},
				{"fbo_10_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[10].ratio	},
				{"fbo_10_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[10].format	},
				{"fbo_10_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[10].numColorAttachments	},
				{"fbo_11_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[11].ratio	},
				{"fbo_11_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[11].format	},
				{"fbo_11_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[11].numColorAttachments	},
				{"fbo_12_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[12].ratio	},
				{"fbo_12_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[12].format	},
				{"fbo_12_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[12].numColorAttachments	},
				{"fbo_13_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[13].ratio	},
				{"fbo_13_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[13].format	},
				{"fbo_13_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[13].numColorAttachments	},
				{"fbo_14_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[14].ratio	},
				{"fbo_14_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[14].format	},
				{"fbo_14_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[14].numColorAttachments	},
				{"fbo_15_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[15].ratio	},
				{"fbo_15_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[15].format	},
				{"fbo_15_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[15].numColorAttachments	},
				{"fbo_16_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[16].ratio	},
				{"fbo_16_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[16].format	},
				{"fbo_16_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[16].numColorAttachments	},
				{"fbo_17_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[17].ratio	},
				{"fbo_17_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[17].format	},
				{"fbo_17_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[17].numColorAttachments	},
				{"fbo_18_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[18].ratio	},
				{"fbo_18_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[18].format	},
				{"fbo_18_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[18].numColorAttachments	},
				{"fbo_19_ratio",			SectionVar::TYPE_INT,			&GLDRV->fboConfig[19].ratio	},
				{"fbo_19_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[19].format	},
				{"fbo_19_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[19].numColorAttachments	},

				{"fbo_20_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[20].width	},
				{"fbo_20_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[20].height	},
				{"fbo_20_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[20].format	},
				{"fbo_20_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[20].numColorAttachments	},
				{"fbo_21_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[21].width	},
				{"fbo_21_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[21].height	},
				{"fbo_21_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[21].format	},
				{"fbo_21_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[21].numColorAttachments	},
				{"fbo_22_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[22].width	},
				{"fbo_22_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[22].height	},
				{"fbo_22_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[22].format	},
				{"fbo_22_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[22].numColorAttachments	},
				{"fbo_23_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[23].width	},
				{"fbo_23_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[23].height	},
				{"fbo_23_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[23].format	},
				{"fbo_23_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[23].numColorAttachments },
				{"fbo_24_width",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[24].width	},
				{"fbo_24_height",			SectionVar::TYPE_FLOAT,			&GLDRV->fboConfig[24].height	},
				{"fbo_24_format",			SectionVar::TYPE_STRING,		&GLDRV->fboConfig[24].format	},
				{"fbo_24_colorAttachments",	SectionVar::TYPE_INT,			&GLDRV->fboConfig[24].numColorAttachments }
			})
	{
	}

	SpoReader::~SpoReader()
	{
	}

	bool SpoReader::readAsciiFromFile(std::string_view filepath)
	{
		m_filepath = filepath;
		std::ifstream file(m_filepath, std::ios::in | std::ios::binary);
		if (file.good()) {
			m_scriptData = { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
			file.close();
			return true;
		}
		else
			return false;
	}

	bool SpoReader::readAsciiFromNetwork(std::string_view sScript)
	{
		m_filepath = "Network";
		m_scriptData = sScript;
		return true;
	}

	int SpoReader::loadScriptData()
	{
		std::istringstream f(m_scriptData);
		std::string line;
		int lineNum = 0; // Line counter

		int sec_id = -1;
		Section* new_sec = NULL;
		Spline* new_spl = NULL;
		SectionCommand command = SectionCommand::INVALID;


		while (std::getline(f, line)) {
			lineNum++;
			// Remove '\r' (if exists)
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			// Ignore comments or empty line
			if (line.empty() || (line[0] == ';') || (line[0] == '\n') || (line[0] == '\r') || (line[0] == ' ') || (line[0] == '\t')) {
				//Logger::info(LogLevel::low, "  Comments found or empty in line %i, ignoring this line.", lineNum);
				continue;
			}

			// If its a section
			if (line[0] == '[') {
				sec_id = loadSectionScriptData();
				return sec_id;
			}
			// Else: it's a config file
			else {
				loadConfigScriptData();
				return 0;
			}
		}
		return -1;
	}

	bool SpoReader::loadConfigScriptData()
	{
		std::istringstream f(m_scriptData);
		std::string line;
		int lineNum = 0; // Line counter
		bool errorsFound = false;

		Logger::ScopedIndent _;

		while (std::getline(f, line)) {
			lineNum++;
			// Remove '\r' (if exists)
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			// Ignore comments or empty line
			if (line.empty() || (line[0] == '\n') || (line[0] == '\r')) {
				continue;
			}
			if ((line[0] == ';') || (line[0] == ' ') || (line[0] == '\t')) {
				//Logger::info(LogLevel::low, "  Comments found or empty in line %i, ignoring this line.", lineNum);
				continue;
			}

			std::pair<std::string, std::string> s_line = splitIn2Lines(line);
			// check if its a known command
			const auto command = scriptCommandFound(s_line.first);
			if (command == -1) {
				Logger::error(
					"'%s' is not a valid SPO script variable. Check file: %s, line: %d",
					s_line.first.c_str(),
					m_filepath.c_str(),
					lineNum
				);
				errorsFound = true;
				continue;
			}

			switch (scriptCommand[command].m_type) {

			case SectionVar::TYPE_BOOL: {
				auto& Data = *reinterpret_cast<int*>(scriptCommand[command].m_pData);
				Data = atoi(s_line.second.c_str());
				Logger::info(
					LogLevel::low,
					"Command found: %s [%s]",
					scriptCommand[command].m_name.c_str(),
					Data ? "true" : "false"
				);
				break;
			}

			case SectionVar::TYPE_INT: {
				auto& Data = *reinterpret_cast<int*>(scriptCommand[command].m_pData);
				Data = atoi(s_line.second.c_str());
				Logger::info(
					LogLevel::low,
					"Command found: %s [%d]",
					scriptCommand[command].m_name.c_str(),
					Data
				);
				break;
			}

			case SectionVar::TYPE_FLOAT: {
				auto& Data = *reinterpret_cast<float*>(scriptCommand[command].m_pData);
				Data = static_cast<float>(atof(s_line.second.c_str()));
				Logger::info(
					LogLevel::low,
					"Command found: %s [%f]",
					scriptCommand[command].m_name.c_str(),
					Data
				);
				break;
			}

			case SectionVar::TYPE_STRING: {
				auto& Data = *reinterpret_cast<std::string*>(scriptCommand[command].m_pData);
				Data = s_line.second;
				Logger::info(
					LogLevel::low,
					"Command found: %s [%s]",
					scriptCommand[command].m_name.c_str(),
					Data
				);
				break;
			}

			default:
				Logger::error(
					"%d is not a valid variable type id.",
					scriptCommand[command].m_type
				);
				errorsFound = true;
				break;
			}
		}

		return !errorsFound;
	}

	int SpoReader::loadSectionScriptData() {
		std::istringstream f(m_scriptData);
		std::string line;
		int lineNum = 0; // Line counter

		int sec_id = -1;
		Section* new_sec = NULL;
		Spline* new_spl = NULL;
		SectionCommand command = SectionCommand::INVALID;


		while (std::getline(f, line)) {
			lineNum++;
			// Remove '\r' (if exists)
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			// Ignore comments or empty line
			if (line.empty() || (line[0] == ';') || (line[0] == '\n') || (line[0] == '\r') || (line[0] == ' ') || (line[0] == '\t')) {
				//Logger::info(LogLevel::low, "  Comments found or empty in line %i, ignoring this line.", lineNum);
				continue;
			}

			// Remove double spaces
			line = removeDoubleSpaces(line);

			// If its a section
			if (line[0] == '[') {
				char c_sec_type[512];
				// First we read the Section type (key)
				if (sscanf(line.c_str(), "[%s]", c_sec_type) != 1)
					throw std::exception();
				c_sec_type[strlen(c_sec_type) - 1] = '\0';
				const auto sec_type = getSectionType(c_sec_type);

				// by default the section is enabled and marked as not loaded
				sec_id = -1;
				sec_id = DEMO->m_sectionManager.addSection(sec_type, "File: " + m_filepath, true);
				if (sec_id != -1) {
					new_sec = DEMO->m_sectionManager.m_section[sec_id];
					Logger::info(LogLevel::low, "  Section %s added!", new_sec->type_str.c_str());
				}
				else {
					Logger::error("Section %s not supported! File skipped", c_sec_type);
					return sec_id;
				}
			}
			// If we have already loaded the section type, then load it's parameters
			else if (sec_id != -1) {
				std::pair<std::string, std::string> s_line = splitIn2Lines(line);
				// check if its a known command
				if (spoSectionCommand.find(s_line.first) == spoSectionCommand.end()) {
					// If its not found, maybe is a normal parameter
					switch (s_line.first[0]) {
					case 'f':
						command = SectionCommand::PARAM;
						break;
					case 's':
						command = SectionCommand::STRING;
						break;
					case 'u':
						command = SectionCommand::UNIFORM;
						break;
					case 'c':
						command = SectionCommand::SPLINE;
						break;
					case 'm':
						command = SectionCommand::MODIFIER;
						break;
					default:
						command = SectionCommand::INVALID;
						break;
					}
				}
				else {
					command = spoSectionCommand.find(s_line.first)->second;
				}
				// If we found the command
				switch (command)
				{
				case SectionCommand::INVALID:
					Logger::error("  Invalid line: %s", line.c_str());
					break;

				case SectionCommand::IDENTIFIER:
					new_sec->identifier = s_line.second;
					Logger::info(LogLevel::low, "  Section id: %s", new_sec->identifier.c_str());
					break;

				case SectionCommand::ENABLED:
					new_sec->enabled = std::stoi(s_line.second);
					Logger::info(LogLevel::low, "  Section enabled state: %i", new_sec->enabled);
					break;

				case SectionCommand::START:
					new_sec->startTime = std::stof(s_line.second);
					Logger::info(LogLevel::low, "  Section Start time: %f", new_sec->startTime);
					break;

				case SectionCommand::END:
					new_sec->endTime = std::stof(s_line.second);
					Logger::info(LogLevel::low, "  Section End time: %f", new_sec->endTime);
					new_sec->duration = new_sec->endTime - new_sec->startTime;
					if (new_sec->duration <= 0)
						Logger::error("Section End time is less or equal than Start timeStart time!");
					break;

				case SectionCommand::LAYER:
					new_sec->layer = std::stoi(s_line.second);
					Logger::info(LogLevel::low, "  Section layer: %i", new_sec->layer);
					break;


				case SectionCommand::BLEND:
				{
					auto blendModes = splitIn2Lines(s_line.second);

					if ((spoBlendFunc.find(blendModes.first) == spoBlendFunc.end()) ||
						(spoBlendFunc.find(blendModes.second) == spoBlendFunc.end())) {
						Logger::error("Invalid blend mode(s) in line: %s", line.c_str());
					}
					else {
						new_sec->sfactor = spoBlendFunc.find(blendModes.first)->second;
						new_sec->dfactor = spoBlendFunc.find(blendModes.second)->second;
						new_sec->hasBlend = true;
						Logger::info(LogLevel::low, "  Section blend mode: source %i and destination %i", new_sec->sfactor, new_sec->dfactor);
					}
				}
				break;

				case SectionCommand::BLEND_EQUATION:
					if (spoBlendEquationFunc.find(s_line.second) == spoBlendEquationFunc.end())
						Logger::error("Invalid blend equation in line: %s", line.c_str());
					else {
						new_sec->blendEquation = spoBlendEquationFunc.find(s_line.second)->second;
						Logger::info(LogLevel::low, "  Section blend equation: %i", new_sec->blendEquation);
					}
					break;

				case SectionCommand::PARAM:
				{
					try {
						float fval = std::stof(s_line.second);
						new_sec->param.push_back(fval);
						Logger::info(LogLevel::low, "  Section parameter: %s = %f", s_line.first.c_str(), fval);
					}
					catch (...) {
						Logger::error("  Parameter not loaded in line: %s --> The parameter: %s with value [%s] could not be parsed", line.c_str(), s_line.first.c_str(), s_line.second.c_str());
					}
				}
				break;

				case SectionCommand::STRING:
					new_sec->strings.push_back(s_line.second);
					Logger::info(LogLevel::low, "  Loaded string: \"%s\"", s_line.second.c_str());
					break;

				case SectionCommand::UNIFORM:
					new_sec->uniform.push_back(s_line.second);
					Logger::info(LogLevel::low, "  Loaded uniform: \"%s\"", s_line.second.c_str());
					break;

				case SectionCommand::SPLINE:
					new_spl = new Spline();
					new_spl->filename = DEMO->m_dataFolder + s_line.second;
					new_spl->duration = new_sec->duration; // Spline duration is the same as the sectio duration
					new_sec->spline.push_back(new_spl);
					Logger::info(LogLevel::low, "  Loaded Spline: %s", new_spl->filename.c_str());
					break;
				default:
					Logger::error("Unknown section variable was found in line: \"%s\"", line.c_str());
					break;
				}
			}
		}

		return sec_id;
	}

	std::pair<std::string, std::string> SpoReader::splitIn2Lines(const std::string& line) {
		std::pair<std::string, std::string> ret("", "");

		std::vector<std::string> strings = splitInMultipleLines(line);

		// recompose the string: in the first we have the commmand, in the second we have all the string
		if (strings.size() > 1) {
			ret.first = strings[0];
			for (int i = 1; i < strings.size(); i++) {
				ret.second += strings[i];
				if ((i + 1) < strings.size())
					ret.second += " ";
			}
		}
		return ret;
	}

	std::vector<std::string> SpoReader::splitInMultipleLines(const std::string& line) {
		std::vector<std::string> strings;
		std::istringstream f(line);
		std::string s;
		// Split the string by spaces
		while (std::getline(f, s, ' ')) {
			strings.push_back(s);
		}
		return strings;
	}

	std::string& SpoReader::removeDoubleSpaces(std::string& line)
	{
		std::string::size_type pos = line.find("  ");

		while (pos != std::string::npos) {

			// replace BOTH spaces with one space
			line.replace(pos, 2, " ");

			// start searching again, where you left off
			// rather than going back to the beginning
			pos = line.find("  ", pos);
		}

		return line;
	}

	int SpoReader::scriptCommandFound(const std::string& command)
	{
		for (int i = 0; i < scriptCommand.size(); i++) {
			if (command == scriptCommand[i].m_name)
				return i;
		}
		return -1;
	}
}
