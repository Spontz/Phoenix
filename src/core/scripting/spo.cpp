// spo.cpp
// Spontz Demogroup

#include "spo.h"

namespace Phoenix {

	SpoReader::SpoReader()
	:
	m_filepath(""),
	m_scriptData("")
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
				LOG->Info(LogLevel::LOW, "  Comments found or empty in line %i, ignoring this line.", lineNum);
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
				LOG->Info(LogLevel::LOW, "  Comments found in line %i, ignoring this line.", lineNum);
				continue;
			}

			std::pair<std::string, std::string> s_line = splitIn2Lines(line);
			// check if its a known command
			command = scriptCommandFound(s_line.first);
			if (command != -1) {
				switch (scriptCommand[command].vType) {
				case SectionVar::TYPE_INT:
					*(int*)((char**)scriptCommand[command].vAddr) = std::stoi(s_line.second);
					iptr = (int*)scriptCommand[command].vAddr;
					LOG->Info(LogLevel::LOW, "  Command found: %s = %d", scriptCommand[command].cName.c_str(), *iptr);
					break;
				case SectionVar::TYPE_FLOAT:
					*(float*)((char**)scriptCommand[command].vAddr) = std::stof(s_line.second);
					fptr = (float*)scriptCommand[command].vAddr;
					LOG->Info(LogLevel::LOW, "  Command found: %s = %f", scriptCommand[command].cName.c_str(), *fptr);
					break;
				case SectionVar::TYPE_STRING:
					*((char**)scriptCommand[command].vAddr) = _strdup(s_line.second.c_str());
					sptr = (char**)scriptCommand[command].vAddr;
					LOG->Info(LogLevel::LOW, "  Command found: %s = %s", scriptCommand[command].cName.c_str(), *sptr);
					break;
				default:
					LOG->Error("%d is not a valid variable type id.", scriptCommand[command].vType);
					break;
				}
			}
			else {
				LOG->Error("'%s' is not a valid SPO script variable. Check file: %s, line: %d", s_line.first.c_str(), m_filepath.c_str(), lineNum);
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
				LOG->Info(LogLevel::LOW, "  Comments found or empty in line %i, ignoring this line.", lineNum);
				continue;
			}

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
				sec_id = DEMO->sectionManager.addSection(sec_type, "File: " + m_filepath, true);
				if (sec_id != -1) {
					LOG->Info(LogLevel::LOW, "  Section %s added!", sec_type.c_str());
					new_sec = DEMO->sectionManager.section[sec_id];
				}
				else {
					LOG->Error("Section %s not supported! File skipped", sec_type.c_str());
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
						LOG->Error("  Invalid line: %s", line.c_str());
						break;

					case SectionCommand_::IDENTIFIER:
						new_sec->identifier = s_line.second;
						LOG->Info(LogLevel::LOW, "  Section id: %s", new_sec->identifier.c_str());
						break;

					case SectionCommand_::ENABLED:
						new_sec->enabled = std::stoi(s_line.second);
						LOG->Info(LogLevel::LOW, "  Section enabled state: %i", new_sec->enabled);
						break;

					case SectionCommand_::START:
						new_sec->startTime = std::stof(s_line.second);
						LOG->Info(LogLevel::LOW, "  Section Start time: %f", new_sec->startTime);
						break;

					case SectionCommand_::END:
						new_sec->endTime = std::stof(s_line.second);
						LOG->Info(LogLevel::LOW, "  Section End time: %f", new_sec->endTime);
						new_sec->duration = new_sec->endTime - new_sec->startTime;
						if (new_sec->duration <= 0)
							LOG->Error("Section End time is less or equal than Start timeStart time!");
						break;

					case SectionCommand_::LAYER:
						new_sec->layer = std::stoi(s_line.second);
						LOG->Info(LogLevel::LOW, "  Section layer: %i", new_sec->layer);
						break;

					
					case SectionCommand_::BLEND:
						{
							auto blendModes = splitIn2Lines(s_line.second);

							if ((spoBlendFunc.find(blendModes.first) == spoBlendFunc.end()) ||
								(spoBlendFunc.find(blendModes.second) == spoBlendFunc.end())) {
								LOG->Error("Invalid blend mode(s) in line: %s", line.c_str());
							}
							else {
								new_sec->sfactor = spoBlendFunc.find(blendModes.first)->second;
								new_sec->dfactor = spoBlendFunc.find(blendModes.second)->second;
								new_sec->hasBlend = true;
								LOG->Info(LogLevel::LOW, "  Section blend mode: source %i and destination %i", new_sec->sfactor, new_sec->dfactor);
							}
						}
						break;

					case SectionCommand_::BLEND_EQUATION:
						if (spoBlendEquationFunc.find(s_line.second) == spoBlendEquationFunc.end())
							LOG->Error("Invalid blend equation in line: %s", line.c_str());
						else {
							new_sec->blendEquation = spoBlendEquationFunc.find(s_line.second)->second;
							LOG->Info(LogLevel::LOW, "  Section blend equation: %i", new_sec->blendEquation);
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
								LOG->Error("Invalid alpha in line: %s", line.c_str());
								break;
							}
							if (spoAlphaFunc.find(alphaValues[0]) == spoAlphaFunc.end())
								LOG->Error("Invalid alpha function in line: %s", line.c_str());
							else {
								new_sec->alphaFunc = spoAlphaFunc.find(alphaValues[0])->second;
								new_sec->hasAlpha = true;
								LOG->Info(LogLevel::LOW, "  Section alpha: from %f to %f", new_sec->alpha1, new_sec->alpha2);
							}
							
						}
						break;
							
					case SectionCommand_::PARAM:
						{
						try{
							float fval = std::stof(s_line.second);
							new_sec->param.push_back(fval);
							LOG->Info(LogLevel::LOW, "  Section parameter: %s = %f", s_line.first.c_str(), fval);
						}
						catch (...)	{
							LOG->Error("  Parameter not loaded in line: %s --> The parameter: %s with value [%s] could not be parsed", line.c_str(), s_line.first.c_str(), s_line.second.c_str());
						}
						}
						break;

					case SectionCommand_::STRING:
						new_sec->strings.push_back(s_line.second);
						LOG->Info(LogLevel::LOW, "  Loaded string: \"%s\"", s_line.second.c_str());
						break;

					case SectionCommand_::UNIFORM:
						new_sec->uniform.push_back(s_line.second);
						LOG->Info(LogLevel::LOW, "  Loaded uniform: \"%s\"", s_line.second.c_str());
						break;

					case SectionCommand_::SPLINE:
						new_spl = new Spline();
						new_spl->filename = DEMO->dataFolder + s_line.second;
						new_spl->duration = new_sec->duration; // Spline duration is the same as the sectio duration
						new_sec->spline.push_back(new_spl);
						LOG->Info(LogLevel::LOW, "  Loaded Spline: %s", new_spl->filename.c_str());
						break;
					default:
						LOG->Error("Unknown section variable was found in line: \"%s\"", line.c_str());
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

	int SpoReader::scriptCommandFound(const std::string& command)
	{
		for (int i = 0; i < scriptCommand.size(); i++) {
			if (command == scriptCommand[i].cName)
				return i;
		}
		return -1;
	}
}
