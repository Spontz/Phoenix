// ShaderVars.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/shadervars.h"

namespace Phoenix {

	ShaderVars::ShaderVars(Section* pSection, SP_Shader const& spShader)
	{
		my_section = pSection;
		my_shader = spShader;
	}

	bool ShaderVars::ReadString(std::string_view StringVar)
	{
		std::vector<std::string>	vars;
		bool	has_properties = false;

		splitString(StringVar.data(), vars, ' ');	// Split the main string by spaces

		if (vars.size() < 3) {
			Logger::error(
				"Error reading Shader Variable [section: {}], format is: 'uniform <var_type> "
				"<var_name> <var_value> <option>', but the string was: 'uniform {}'",
				my_section->type_str,
				StringVar
			);
			return false;
		}

		std::string var_type = vars[0];
		std::string var_name = vars[1];
		std::string var_value = vars[2];
		std::vector<std::string> var_properties;
		if (vars.size() > 3) {
			// In case is a sampler, load the properties
			if (var_type == "sampler2D" || var_type == "samplerCube") {
				has_properties = true;
				// Load all properties
				for (int i = 3; i < vars.size(); i++) {
					var_properties.emplace_back(vars[i]);
				}
			}
			// In case is not a sampler, then we concatenate all the values
			else {
				for (int i = 3; i < vars.size(); i++) {
					var_value += vars[i];
				}
			}
			
		}

		if (has_properties) {
			// Concatenate all properties in a single string
			std::string concatProperties;
			for (const auto& str : var_properties)
				concatProperties += str + ",";

			Logger::info(LogLevel::med, "Shader Variable read [section: {}, shader gl_id: {}]: type [{}], name [{}], value [{}], properties [{}]", my_section->type_str, my_shader->getId(), var_type, var_name, var_value, concatProperties);
		}
		else
			Logger::info(LogLevel::med, "Shader Variable read [section: {}, shader gl_id: {}]: type [{}], name [{}], value [{}]", my_section->type_str, my_shader->getId(), var_type, var_name, var_value);

		VarType varTypeValue = getVarType(var_type);
		switch (varTypeValue) {

		case VarType::FLOAT:
		{
			auto var = std::make_shared<varFloat>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value);
			var->eva->compileFormula();
			vfloat.emplace_back(var);
		}
		break;

		case VarType::VEC2:
		{
			auto var = std::make_shared<varVec2>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value[0]);
			var->eva->SymbolTable.add_variable("v2", var->value[1]);
			var->eva->compileFormula();
			vec2.emplace_back(var);
		}
		break;

		case VarType::VEC3:
		{
			auto var = std::make_shared<varVec3>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value[0]);
			var->eva->SymbolTable.add_variable("v2", var->value[1]);
			var->eva->SymbolTable.add_variable("v3", var->value[2]);
			var->eva->compileFormula();
			vec3.emplace_back(var);
		}
		break;

		case VarType::VEC4:
		{
			auto var = std::make_shared<varVec4>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value[0]);
			var->eva->SymbolTable.add_variable("v2", var->value[1]);
			var->eva->SymbolTable.add_variable("v3", var->value[2]);
			var->eva->SymbolTable.add_variable("v4", var->value[3]);
			var->eva->compileFormula();
			vec4.emplace_back(var);
		}
		break;
		
		case VarType::MAT3:
		{
			auto var = std::make_shared<varMat3>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value[0][0]);
			var->eva->SymbolTable.add_variable("v2", var->value[0][1]);
			var->eva->SymbolTable.add_variable("v3", var->value[0][2]);
			var->eva->SymbolTable.add_variable("v4", var->value[1][0]);
			var->eva->SymbolTable.add_variable("v5", var->value[1][1]);
			var->eva->SymbolTable.add_variable("v6", var->value[1][2]);
			var->eva->SymbolTable.add_variable("v7", var->value[2][0]);
			var->eva->SymbolTable.add_variable("v8", var->value[2][1]);
			var->eva->SymbolTable.add_variable("v9", var->value[2][2]);
			var->eva->compileFormula();
			mat3.emplace_back(var);
		}
		break;
		
		case VarType::MAT4:
		{
			auto var = std::make_shared<varMat4>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value[0][0]);
			var->eva->SymbolTable.add_variable("v2", var->value[0][1]);
			var->eva->SymbolTable.add_variable("v3", var->value[0][2]);
			var->eva->SymbolTable.add_variable("v4", var->value[0][3]);
			var->eva->SymbolTable.add_variable("v5", var->value[1][0]);
			var->eva->SymbolTable.add_variable("v6", var->value[1][1]);
			var->eva->SymbolTable.add_variable("v7", var->value[1][2]);
			var->eva->SymbolTable.add_variable("v8", var->value[1][3]);
			var->eva->SymbolTable.add_variable("v9", var->value[2][0]);
			var->eva->SymbolTable.add_variable("v10", var->value[2][1]);
			var->eva->SymbolTable.add_variable("v11", var->value[2][2]);
			var->eva->SymbolTable.add_variable("v12", var->value[2][3]);
			var->eva->SymbolTable.add_variable("v13", var->value[3][0]);
			var->eva->SymbolTable.add_variable("v14", var->value[3][1]);
			var->eva->SymbolTable.add_variable("v15", var->value[3][2]);
			var->eva->SymbolTable.add_variable("v16", var->value[3][3]);
			var->eva->compileFormula();
			mat4.emplace_back(var);
		}
		break;
		
		case VarType::SAMPLER2D:
		{
			// if it's an FBO...
			if (var_value.substr(0, 3) == "fbo")
			{
				auto const fboNum = std::stoi(var_value.substr(3));
				bool sendAllAttachments = true; // Bu default, we will send all the fbo attachments
				int32_t colorAttachmentToSend = -1; // Stores the color attachment that will be sent. -1:all attachments

				if (fboNum<0 || fboNum>(FBO_BUFFERS - 1)) {
					Logger::error("Section {}: sampler2D {} not correct, it should be 'fboX', where X=>0 and X<={}", my_section->identifier, var_value, (FBO_BUFFERS - 1));
					return false;
				}
				
				// Read the FBO properties
				for (auto const& prop : var_properties) {
					if (!loadFboProperty(colorAttachmentToSend, prop))
						Logger::error("Section {}: sampler2D {} has a non recognized property: {}", my_section->identifier, var_value, prop);
				}

				// Get the total FBO attachments available
				int fboAttachments = DEMO->m_fboManager.fbo[fboNum]->numAttachments;

				// If we need to send a specific attachment...
				if (colorAttachmentToSend >= 0) {
					sendAllAttachments = false;

					// Validate that the attachment is valid
					if (colorAttachmentToSend > (fboAttachments - 1)) {
						Logger::error("Section {}: sampler2D {}, attachment ({}) is not available, only attachment 0 has been sent to the shader", my_section->identifier, var_value, colorAttachmentToSend);
						colorAttachmentToSend = 0;
					}
				}

				if (sendAllAttachments) {
					// Send all FBO attachments
					for (int32_t i = 0; i < fboAttachments; i++)
					{
						auto var = std::make_shared<varSampler2D>();

						var->isFBO = true;
						var->fboNum = fboNum;
						var->fboAttachment = i;
						var->name = var_name;
						if (fboAttachments>1)
							var->name += "[" + std::to_string(i) + "]";

						var->loc = my_shader->getUniformLocation(var->name.c_str());
						var->texUnitID = static_cast<int>(sampler2D.size());
						sampler2D.emplace_back(var);
					}
				}
				else {
					// Send only one specific FBO attcahment
					auto var = std::make_shared<varSampler2D>();

					var->isFBO = true;
					var->fboNum = fboNum;
					var->fboAttachment = colorAttachmentToSend;
					var->name = var_name;

					var->loc = my_shader->getUniformLocation(var->name.c_str());
					var->texUnitID = static_cast<int>(sampler2D.size());
					sampler2D.emplace_back(var);
				}
			}
			else // If it's a normal texture....
			{
				auto var = std::make_shared<varSampler2D>();
				var->name = var_name;
				var->loc = my_shader->getUniformLocation(var->name.c_str());
				var->texUnitID = static_cast<int>(sampler2D.size());
				var->isFBO = false;

				Texture::Properties texProperties;
				// Load texture properties
				for (auto const& prop : var_properties) {
					if (!loadTextureProperty(texProperties, prop))
						Logger::error("Section {}: sampler2D has a non recognized property: {}", my_section->identifier, prop);
				}

				var->texture = DEMO->m_textureManager.addTexture(DEMO->m_dataFolder + var_value, texProperties);
				if (var->texture) // If texture is valid
					sampler2D.emplace_back(var);
			}
		}
		break;
			
		case VarType::SAMPLERCUBE:
		{
			auto var = std::make_shared<varSamplerCube>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->cubemapUnitID = static_cast<int>(samplerCube.size());

			Texture::Properties texProperties;
			// Load texture properties
			for (auto const& prop : var_properties) {
				if (!loadTextureProperty(texProperties, prop))
					Logger::error("Section {}: samplerCube has a non recognized property: {}", my_section->identifier, prop);
			}

			// Check if images exist
			std::string samplerCubePath = DEMO->m_dataFolder + var_value + "/";
			const std::vector<std::string> samplerCubeImages = { "right.jpg","left.jpg","top.jpg","bottom.jpg","front.jpg","back.jpg" };
			std::vector<std::string> samplerCubePaths;
			for (auto& s : samplerCubeImages) {
				samplerCubePaths.emplace_back(samplerCubePath + s);
			}

			bool imagesOK = true;
			for (auto& s : samplerCubePaths) {
				if (!Utils::checkFileExists(s)) {
					Logger::error("Section {}: samplerCube is missing the image: {}", my_section->identifier, s);
					imagesOK = false;
				}
			}
			if (imagesOK) {
				var->cubemap = DEMO->m_textureManager.addCubemap(samplerCubePaths, texProperties.m_flip);
				if (var->cubemap) // If texture is valid
					samplerCube.emplace_back(var);
			}
		}
		break;

		case VarType::UNKNOWN:
			Logger::error("Variable type {} is not supported.", var_type);
			break;
		
		}
		
		return true;
	}

	// Set all the shader values. Please note that the shader must be used before!!
	void ShaderVars::setValues()
	{
		for (auto& var : vfloat) {
			var->eva->executeFormula();
			my_shader->setValue(var->name.c_str(), var->value);
		}
		
		for (auto& var : vec2) {
			var->eva->executeFormula();
			my_shader->setValue(var->name.c_str(), var->value);
		}

		for (auto& var : vec3) {
			var->eva->executeFormula();
			my_shader->setValue(var->name.c_str(), var->value);
		}
		for (auto& var : vec4) {
			var->eva->executeFormula();
			my_shader->setValue(var->name.c_str(), var->value);
		}

		for (auto& var : mat3) {
			var->eva->executeFormula();
			my_shader->setValue(var->name.c_str(), var->value);
		}

		for (auto& var : mat4) {
			var->eva->executeFormula();
			my_shader->setValue(var->name.c_str(), var->value);
		}

		for (auto& var : sampler2D) {
			my_shader->setValue(var->name.c_str(), var->texUnitID);
			// This ugly and dirty. This is needed because when we rezise the screen, the FBO's are
			// recalculated (therefore texGLid is changed), therefoere we need to look everytime if
			// the 'texture GL id' has changed
			if (var->isFBO) {
				glBindTextureUnit(var->texUnitID, DEMO->m_fboManager.getOpenGLTextureID(var->fboNum,var->fboAttachment));
			}
			else
				glBindTextureUnit(var->texUnitID, var->texture->m_textureID);
		}

		for (auto& var : samplerCube) {
			my_shader->setValue(var->name.c_str(), var->cubemapUnitID);
			var->cubemap->bind();
			//glBindTextureUnit(var->cubemapUnitID, var->cubemap->m_cubemapID);
			//Logger::info(LogLevel::high, "Cubemap num. {} of {}", i, samplerCube.size());
			//Logger::info(LogLevel::high, "Cubemap name, Texture Unit, cubeMapID: {},{},{}", var->name.c_str(), var->cubemapUnitID, var->cubemap->m_cubemapID);
		}
	}

	void ShaderVars::validateValues(std::string_view sectionID)
	{
		for (auto& var : vfloat) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform float {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}

		for (auto& var : vec2) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform vec2 {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}

		for (auto& var : vec3) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform vec3 {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}
		for (auto& var : vec4) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform vec4 {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}

		for (auto& var : mat3) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform mat3 {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}

		for (auto& var : mat4) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform mat4 {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}

		for (auto& var : sampler2D) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform sampler2D {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}

		for (auto& var : samplerCube) {
			if (-1 == my_shader->getUniformLocation(var->name))
				Logger::error("Variable 'uniform samplerCube {}' not found in section {}, shader '{}'", var->name, sectionID, my_shader->getURI());
		}
	}

	void ShaderVars::validateAndSetValues(std::string_view sectionID)
	{
		validateValues(sectionID);
		setValues();
	}

	// Splits a stirng in several strings, splitted by character 'ch'
	size_t ShaderVars::splitString(const std::string& txt, std::vector<std::string>& strs, char ch)
	{
		size_t pos = txt.find(ch);
		size_t initialPos = 0;
		strs.clear();

		// Decompose statement
		while (pos != std::string::npos) {
			strs.emplace_back(txt.substr(initialPos, pos - initialPos));
			initialPos = pos + 1;

			pos = txt.find(ch, initialPos);
		}

		// Add the last one
		strs.emplace_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

		return strs.size();
	}

	bool ShaderVars::loadTextureProperty(Texture::Properties& texProperty, const std::string& property)
	{
		if (property ==  "FILTER:NONE") {
			texProperty.m_useLinearFilter = false;
			return true;
		}
		else if (property == "FILTER:LINEAR") {
			texProperty.m_useLinearFilter = true;
			return true;
		}
		else if (property == "FLIP:TRUE") {
			texProperty.m_flip = true;
			return true;
		}
		else if (property == "FLIP:FALSE") {
			texProperty.m_flip = false;
			return true;
		}

		return false; // Property was not recognized
	}


	bool ShaderVars::loadFboProperty(int32_t& colorAttachments, const std::string& property)
	{
		bool loaded = false;
		if (property.rfind("ATTACHMENT:", 0) == 0) {
			size_t pos = property.find_first_of(":");
			std::string attachmentsStr = property.substr(pos + 1);
			try {
				uint32_t att = static_cast<uint32_t>(std::stoi(attachmentsStr));
				colorAttachments = att;
				loaded = true;
			}
			catch (std::exception &e) {
				Logger::error("Property value not recognized: found {}, expected a number. Error: {}", attachmentsStr, e.what());
				loaded = false;
			}
		}
		return loaded;
	}

	ShaderVars::VarType ShaderVars::getVarType(std::string_view varTypeStr) const
	{
		auto it = m_VarTypeMap.find(std::string(varTypeStr));
		if (it != m_VarTypeMap.end())
			return it->second;
		else
			return VarType::UNKNOWN;
	}
}
