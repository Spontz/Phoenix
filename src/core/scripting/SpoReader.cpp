// SpoReader.cpp
// Spontz Demogroup

#include "core/scripting/SpoReader.h"

namespace Phoenix {

	SpoReader::SpoReader()
	:
	m_filepath(""),
	m_scriptData(""),
	scriptCommand ({
			{"demo_name",				SectionVar::TYPE_STRING,		&DEMO->m_demoName		},
			{"debug",					SectionVar::TYPE_BOOL,			&DEMO->m_debug			},
			{"debugFontSize",			SectionVar::TYPE_FLOAT,			&DEMO->m_debug_fontSize	},
			{"loop",					SectionVar::TYPE_BOOL,			&DEMO->m_loop			},
			{"sound",					SectionVar::TYPE_BOOL,			&DEMO->m_sound			},
			{"demo_start",				SectionVar::TYPE_FLOAT,			&DEMO->m_demoStartTime	},
			{"demo_end",				SectionVar::TYPE_FLOAT,			&DEMO->m_demoEndTime	},
			{"slave",					SectionVar::TYPE_BOOL,			&DEMO->m_slaveMode		},
			{"log_detail",				SectionVar::TYPE_INT,			&DEMO->m_logLevel		},

			{"gl_fullscreen",			SectionVar::TYPE_INT,			&GLDRV->config.fullScreen				},
			{"gl_width",				SectionVar::TYPE_INT,			&GLDRV->config.framebuffer_width		},
			{"gl_height",				SectionVar::TYPE_INT,			&GLDRV->config.framebuffer_height		},
			{"gl_aspect",				SectionVar::TYPE_FLOAT,			&GLDRV->config.framebuffer_aspect_ratio	},
			{"gl_stencil",				SectionVar::TYPE_INT,			&GLDRV->config.stencil					},
			{"gl_vsync",				SectionVar::TYPE_INT,			&GLDRV->config.vsync					},
			{"gl_multisample",			SectionVar::TYPE_INT,			&GLDRV->config.multisampling			},

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
	})
	{
	}

	SpoReader::~SpoReader()
	{
	}

	bool SpoReader::readAsciiFromFile(const std::string& filepath)
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

	bool SpoReader::readAsciiFromNetwork(std::string sScript)
	{
		m_filepath = "Netowrk";
		m_scriptData = sScript;
		return true;
	}

	int SpoReader::loadScriptData()
	{
		std::istringstream f(m_scriptData);
		std::string line;
		int lineNum = 0; // Line counter
		std::string sec_type;
		int sec_id = -1;
		Section* new_sec = NULL;
		Spline* new_spl = NULL;
		SectionCommand_ command = SectionCommand_::INVALID;


		while (std::getline(f, line)) {
			lineNum++;
			// Remove '\r' (if exists)
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			// Ignore comments or empty line
			if (line.empty() || (line[0] == ';') || (line[0] == '\n') || (line[0] == '\r') || (line[0] == ' ') || (line[0] == '\t')) {
				Logger::info(LogLevel::low, "  Comments found or empty in line %i, ignoring this line.", lineNum);
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
		std::string sec_type;
		Section* new_sec = NULL;
		Spline* new_spl = NULL;
		int command = -1;
		// generic variable loading
		int* iptr;
		float* fptr;
		char** sptr;


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
				Logger::info(LogLevel::low, "  Comments found in line %i, ignoring this line.", lineNum);
				continue;
			}

			std::pair<std::string, std::string> s_line = splitIn2Lines(line);
			// check if its a known command
			command = scriptCommandFound(s_line.first);
			if (command != -1) {
				switch (scriptCommand[command].vType) {
				case SectionVar::TYPE_BOOL:
					*(bool*)((char**)scriptCommand[command].vAddr) = std::stoi(s_line.second);
					iptr = (int*)scriptCommand[command].vAddr;
					Logger::info(LogLevel::low, "  Command found: %s = %d", scriptCommand[command].cName.c_str(), *iptr);
					break;
				case SectionVar::TYPE_INT:
					*(int*)((char**)scriptCommand[command].vAddr) = std::stoi(s_line.second);
					iptr = (int*)scriptCommand[command].vAddr;
					Logger::info(LogLevel::low, "  Command found: %s = %d", scriptCommand[command].cName.c_str(), *iptr);
					break;
				case SectionVar::TYPE_FLOAT:
					*(float*)((char**)scriptCommand[command].vAddr) = std::stof(s_line.second);
					fptr = (float*)scriptCommand[command].vAddr;
					Logger::info(LogLevel::low, "  Command found: %s = %f", scriptCommand[command].cName.c_str(), *fptr);
					break;
				case SectionVar::TYPE_STRING:
					*((char**)scriptCommand[command].vAddr) = _strdup(s_line.second.c_str());
					sptr = (char**)scriptCommand[command].vAddr;
					Logger::info(LogLevel::low, "  Command found: %s = %s", scriptCommand[command].cName.c_str(), *sptr);
					break;
				default:
					Logger::error("%d is not a valid variable type id.", scriptCommand[command].vType);
					break;
				}
			}
			else {
				Logger::error("'%s' is not a valid SPO script variable. Check file: %s, line: %d", s_line.first.c_str(), m_filepath.c_str(), lineNum);
			}
		}
		return true;
	}

	int SpoReader::loadSectionScriptData() {
		std::istringstream f(m_scriptData);
		std::string line;
		int lineNum = 0; // Line counter
		std::string sec_type;
		int sec_id = -1;
		Section* new_sec = NULL;
		Spline* new_spl = NULL;
		SectionCommand_ command = SectionCommand_::INVALID;


		while (std::getline(f, line)) {
			lineNum++;
			// Remove '\r' (if exists)
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			// Ignore comments or empty line
			if (line.empty() || (line[0] == ';') || (line[0] == '\n') || (line[0] == '\r') || (line[0] == ' ') || (line[0] == '\t')) {
				Logger::info(LogLevel::low, "  Comments found or empty in line %i, ignoring this line.", lineNum);
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
				sec_type = c_sec_type;
				sec_type.erase(sec_type.size() - 1);

				// by default the section is enabled and marked as not loaded
				sec_id = -1;
				sec_id = DEMO->m_sectionManager.addSection(sec_type, "File: " + m_filepath, true);
				if (sec_id != -1) {
					Logger::info(LogLevel::low, "  Section %s added!", sec_type.c_str());
					new_sec = DEMO->m_sectionManager.section[sec_id];
				}
				else {
					Logger::error("Section %s not supported! File skipped", sec_type.c_str());
					return sec_id;
				}
			}
			// If we have already loaded the section type, then load it's parameters
			else if (sec_id != -1) {
				std::pair<std::string, std::string> s_line = splitIn2Lines(line);
				// check if its a known command
				if (spoSectionCommand_.find(s_line.first) == spoSectionCommand_.end()) {
					// If its not found, maybe is a normal parameter
					switch (s_line.first[0]) {
					case 'f':
						command = SectionCommand_::PARAM;
						break;
					case 's':
						command = SectionCommand_::STRING;
						break;
					case 'u':
						command = SectionCommand_::UNIFORM;
						break;
					case 'c':
						command = SectionCommand_::SPLINE;
						break;
					case 'm':
						command = SectionCommand_::MODIFIER;
						break;
					default:
						command = SectionCommand_::INVALID;
						break;
					}
				}
				else {
					command = spoSectionCommand_.find(s_line.first)->second;
				}
				// If we found the command
				switch (command)
				{
					case SectionCommand_::INVALID:
						Logger::error("  Invalid line: %s", line.c_str());
						break;

					case SectionCommand_::IDENTIFIER:
						new_sec->identifier = s_line.second;
						Logger::info(LogLevel::low, "  Section id: %s", new_sec->identifier.c_str());
						break;

					case SectionCommand_::ENABLED:
						new_sec->enabled = std::stoi(s_line.second);
						Logger::info(LogLevel::low, "  Section enabled state: %i", new_sec->enabled);
						break;

					case SectionCommand_::START:
						new_sec->startTime = std::stof(s_line.second);
						Logger::info(LogLevel::low, "  Section Start time: %f", new_sec->startTime);
						break;

					case SectionCommand_::END:
						new_sec->endTime = std::stof(s_line.second);
						Logger::info(LogLevel::low, "  Section End time: %f", new_sec->endTime);
						new_sec->duration = new_sec->endTime - new_sec->startTime;
						if (new_sec->duration <= 0)
							Logger::error("Section End time is less or equal than Start timeStart time!");
						break;

					case SectionCommand_::LAYER:
						new_sec->layer = std::stoi(s_line.second);
						Logger::info(LogLevel::low, "  Section layer: %i", new_sec->layer);
						break;

					
					case SectionCommand_::BLEND:
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

					case SectionCommand_::BLEND_EQUATION:
						if (spoBlendEquationFunc.find(s_line.second) == spoBlendEquationFunc.end())
							Logger::error("Invalid blend equation in line: %s", line.c_str());
						else {
							new_sec->blendEquation = spoBlendEquationFunc.find(s_line.second)->second;
							Logger::info(LogLevel::low, "  Section blend equation: %i", new_sec->blendEquation);
						}
						break;
							
					case SectionCommand_::ALPHA:
						{
							auto alphaValues = splitInMultipleLines(s_line.second);
							switch (alphaValues.size()) {
							case 2:
								new_sec->alpha1 = new_sec->alpha2 = std::stof(alphaValues[1]);
								break;
							case 3:
								new_sec->alpha1 = std::stof(alphaValues[1]);
								new_sec->alpha2 = std::stof(alphaValues[2]);
								break;
							default:
								Logger::error("Invalid alpha in line: %s", line.c_str());
								break;
							}
							if (spoAlphaFunc.find(alphaValues[0]) == spoAlphaFunc.end())
								Logger::error("Invalid alpha function in line: %s", line.c_str());
							else {
								new_sec->alphaFunc = spoAlphaFunc.find(alphaValues[0])->second;
								new_sec->hasAlpha = true;
								Logger::info(LogLevel::low, "  Section alpha: from %f to %f", new_sec->alpha1, new_sec->alpha2);
							}
							
						}
						break;
							
					case SectionCommand_::PARAM:
						{
						try{
							float fval = std::stof(s_line.second);
							new_sec->param.push_back(fval);
							Logger::info(LogLevel::low, "  Section parameter: %s = %f", s_line.first.c_str(), fval);
						}
						catch (...)	{
							Logger::error("  Parameter not loaded in line: %s --> The parameter: %s with value [%s] could not be parsed", line.c_str(), s_line.first.c_str(), s_line.second.c_str());
						}
						}
						break;

					case SectionCommand_::STRING:
						new_sec->strings.push_back(s_line.second);
						Logger::info(LogLevel::low, "  Loaded string: \"%s\"", s_line.second.c_str());
						break;

					case SectionCommand_::UNIFORM:
						new_sec->uniform.push_back(s_line.second);
						Logger::info(LogLevel::low, "  Loaded uniform: \"%s\"", s_line.second.c_str());
						break;

					case SectionCommand_::SPLINE:
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

	std::pair<std::string, std::string> SpoReader::splitIn2Lines(const std::string & line) {
		std::pair<std::string, std::string> ret("","");

		std::vector<std::string> strings = splitInMultipleLines(line);
		
		// recompose the string: in the first we have the commmand, in the second we have all the string
		if (strings.size() > 1) {
			ret.first = strings[0];
			for (int i=1; i<strings.size(); i++) {
				ret.second += strings[i];
				if ((i+1)<strings.size())
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
			if (command == scriptCommand[i].cName)
				return i;
		}
		return -1;
	}
}
