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

	bool ShaderVars::ReadString(const char* string_var)
	{
		// std::string	var_name, var_type, var_value;

		std::vector<std::string>	vars;

		splitString(string_var, vars, ' ');	// Split the main string by spaces

		if (vars.size() != 3) {
			Logger::error(
				"Error reading Shader Variable [section: %s], format is: 'uniform <var_type> "
				"<var_name> <var_value>', but the string was: 'uniform %s'",
				my_section->type_str.c_str(),
				string_var
			);
			return false;
		}

		auto var_type = vars[0];
		auto var_name = vars[1];
		auto var_value = vars[2];

		Logger::info(
			LogLevel::med,
			"Shader Variable read [section: %s, shader gl_id: %d]: type [%s], name [%s], "
			"value [%s]",
			my_section->type_str.c_str(),
			my_shader->getId(),
			var_type.c_str(),
			var_name.c_str(),
			var_value.c_str()
		);


		if (var_type == "float")	// FLOAT detected
		{
			auto var = std::make_shared<varFloat>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value);
			var->eva->compileFormula();
			vfloat.push_back(var);
		}
		else if (var_type == "vec2")	// VEC2 detected
		{
			auto var = std::make_shared<varVec2>();
			var->name = var_name;
			var->loc = my_shader->getUniformLocation(var->name.c_str());
			var->eva = std::make_shared<MathDriver>(my_section);
			var->eva->expression = var_value;
			var->eva->SymbolTable.add_variable("v1", var->value[0]);
			var->eva->SymbolTable.add_variable("v2", var->value[1]);
			var->eva->compileFormula();
			vec2.push_back(var);
		}
		else if (var_type == "vec3")	// VEC3 detected
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
			vec3.push_back(var);
		}
		else if (var_type == "vec4")	// VEC4 detected
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
			vec4.push_back(var);
		}
		else if (var_type == "mat3")	// MAT3 detected
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
			mat3.push_back(var);
		}
		else if (var_type == "mat4")	// MAT4 detected
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

			mat4.push_back(var);
		}
		else if (var_type == "sampler2D")	// Texture (sampler2D) detected
		{
			if (var_value.substr(0, 3) == "fbo") // If it's an fbo
			{
				auto const fboNum = std::stoi(var_value.substr(3));

				if (fboNum<0 || fboNum>(FBO_BUFFERS - 1)) {
					Logger::error("Section %s: sampler2D fbo not correct, it should be 'fboX', where X=>0 and X<=%d, you choose: %s", this->my_section->identifier.c_str(), (FBO_BUFFERS - 1), var_value);
					return false;
				}
				int fboAttachments = DEMO->m_fboManager.fbo[fboNum]->numAttachments;
				for (int i = 0; i < fboAttachments; i++)
				{
					auto var = std::make_shared<varSampler2D>();

					var->isFBO = true;
					var->fboNum = fboNum;
					var->fboAttachment = i;
					var->name = var_name;

					if (fboAttachments > 1)
						var->name += "[" + std::to_string(i) + "]";

					var->loc = my_shader->getUniformLocation(var->name.c_str());
					var->texUnitID = static_cast<int>(sampler2D.size());
					sampler2D.push_back(var);
				}
			}
			else // If it's a normal texture....
			{
				auto var = std::make_shared<varSampler2D>();
				var->name = var_name;
				var->loc = my_shader->getUniformLocation(var->name.c_str());
				var->texUnitID = static_cast<int>(sampler2D.size());
				var->isFBO = false;
				var->texture = DEMO->m_textureManager.addTexture(DEMO->m_dataFolder + var_value);
				if (var->texture) // If texture is valid
					sampler2D.push_back(var);
			}
		}
		return true;
	}

	// Set all the shader values. Please note that the shader must be used before!!
	void ShaderVars::setValues()
	{
		unsigned int i = 0;
		std::shared_ptr<varFloat> my_vfloat;
		std::shared_ptr<varVec2> my_vec2;
		std::shared_ptr<varVec3> my_vec3;
		std::shared_ptr<varVec4> my_vec4;
		std::shared_ptr<varMat3> my_mat3;
		std::shared_ptr<varMat4> my_mat4;
		std::shared_ptr<varSampler2D> my_sampler2D;

		// TODO: Optimize and remove the ".size()" evaluation. Replace for something more optimal
		for (i = 0; i < vfloat.size(); i++) {
			my_vfloat = vfloat[i];
			my_vfloat->eva->Expression.value();
			my_shader->setValue(my_vfloat->name.c_str(), my_vfloat->value);
		}

		for (i = 0; i < vec2.size(); i++) {
			my_vec2 = vec2[i];
			my_vec2->eva->Expression.value();
			my_shader->setValue(my_vec2->name.c_str(), my_vec2->value);
		}

		for (i = 0; i < vec3.size(); i++) {
			my_vec3 = vec3[i];
			my_vec3->eva->Expression.value();
			my_shader->setValue(my_vec3->name.c_str(), my_vec3->value);
		}
		for (i = 0; i < vec4.size(); i++) {
			my_vec4 = vec4[i];
			my_vec4->eva->Expression.value();
			my_shader->setValue(my_vec4->name.c_str(), my_vec4->value);
		}

		for (i = 0; i < mat3.size(); i++) {
			my_mat3 = mat3[i];
			my_mat3->eva->Expression.value();
			my_shader->setValue(my_mat3->name.c_str(), my_mat3->value);
		}

		for (i = 0; i < mat4.size(); i++) {
			my_mat4 = mat4[i];
			my_mat4->eva->Expression.value();
			my_shader->setValue(my_mat4->name.c_str(), my_mat4->value);
		}

		for (i = 0; i < sampler2D.size(); i++) {
			my_sampler2D = sampler2D[i];
			my_shader->setValue(my_sampler2D->name.c_str(), my_sampler2D->texUnitID);
			// TODO: This ugly and dirty. This is needed because when we rezise the screen, the FBO's are recalculated (therefore texGLid is changed), therefoere we need to look everytime if the texGLid id has changed
			if (my_sampler2D->isFBO) {
				//Fbo* my_fbo = DEMO->fboManager.fbo[my_sampler2D->fboNum];
				//glBindTextureUnit(my_sampler2D->texUnitID, my_fbo->m_colorAttachment[0]);
				glBindTextureUnit(my_sampler2D->texUnitID, DEMO->m_fboManager.getOpenGLTextureID(my_sampler2D->fboNum, my_sampler2D->fboAttachment));
			}
			else
				glBindTextureUnit(my_sampler2D->texUnitID, my_sampler2D->texture->m_textureID);
		}
	}

	// Splits a stirng in several strings, splitted by character 'ch'
	size_t ShaderVars::splitString(const std::string& txt, std::vector<std::string>& strs, char ch)
	{
		size_t pos = txt.find(ch);
		size_t initialPos = 0;
		strs.clear();

		// Decompose statement
		while (pos != std::string::npos) {
			strs.push_back(txt.substr(initialPos, pos - initialPos));
			initialPos = pos + 1;

			pos = txt.find(ch, initialPos);
		}

		// Add the last one
		strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

		return strs.size();
	}
}