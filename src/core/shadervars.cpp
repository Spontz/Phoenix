// shadervars.cpp
// Spontz Demogroup


#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "main.h"
#include "core/shader.h"
#include "core/shadervars.h"




ShaderVars::ShaderVars(Section* sec, Shader* shad)
{
	this->my_section = sec;
	this->my_shader = shad;
}

bool ShaderVars::ReadString(const char * string_var)
{
	char	var_name[MAXSIZE_VAR_NAME];
	char	var_type[MAXSIZE_VAR_TYPE];
	char	var_value[MAXSIZE_VAR_EVAL];
	string s_demo = DEMO->demoDir;

	sscanf(string_var, "%s %s %s", var_type, var_name, var_value);	// Read one string and store values on temporary strings for further evaluation
	LOG->Info(LOG_MED, "Shader Variable read [section: %s, shader gl_id: %d]: string_type [%s], string_name [%s], string_value [%s]", my_section->type_str.c_str(), my_shader->ID, var_type, var_name, var_value);

	if (strcmp(var_type, "float") == 0)	// FLOAT detected
	{
		varFloat *var = new varFloat();
		strcpy(var->name, var_name);
		var->loc = my_shader->getUniformLocation(var->name);
		var->eva = new mathDriver(this->my_section);
		var->eva->expression = var_value;
		var->eva->SymbolTable.add_variable("v1", var->value);
		var->eva->compileFormula();
		vfloat.push_back(var);
	}
	else if (strcmp(var_type, "vec2") == 0)	// VEC2 detected
	{
		varVec2 *var = new varVec2();
		strcpy(var->name, var_name);
		var->loc = my_shader->getUniformLocation(var->name);
		var->eva = new mathDriver(this->my_section);
		var->eva->expression = var_value;
		var->eva->SymbolTable.add_variable("v1", var->value[0]);
		var->eva->SymbolTable.add_variable("v2", var->value[1]);
		var->eva->compileFormula();
		vec2.push_back(var);
	}
	else if (strcmp(var_type, "vec3") == 0)	// VEC3 detected
	{
		varVec3 *var = new varVec3();
		strcpy(var->name, var_name);
		var->loc = my_shader->getUniformLocation(var->name);
		var->eva = new mathDriver(this->my_section);
		var->eva->expression = var_value;
		var->eva->SymbolTable.add_variable("v1", var->value[0]);
		var->eva->SymbolTable.add_variable("v2", var->value[1]);
		var->eva->SymbolTable.add_variable("v3", var->value[2]);
		var->eva->compileFormula();
		vec3.push_back(var);
	}
	else if (strcmp(var_type, "vec4") == 0)	// VEC4 detected
	{
		varVec4 *var = new varVec4();
		strcpy(var->name, var_name);
		var->loc = my_shader->getUniformLocation(var->name);
		var->eva = new mathDriver(this->my_section);
		var->eva->expression = var_value;
		var->eva->SymbolTable.add_variable("v1", var->value[0]);
		var->eva->SymbolTable.add_variable("v2", var->value[1]);
		var->eva->SymbolTable.add_variable("v3", var->value[2]);
		var->eva->SymbolTable.add_variable("v4", var->value[3]);
		var->eva->compileFormula();
		vec4.push_back(var);
	}
	else if (strcmp(var_type, "mat3") == 0)	// MAT3 detected
	{
		varMat3 *var = new varMat3();
		strcpy(var->name, var_name);
		var->loc = my_shader->getUniformLocation(var->name);
		var->eva = new mathDriver(this->my_section);
		var->eva->expression = var_value;
		var->eva->SymbolTable.add_variable("v1", var->value[0]);
		var->eva->SymbolTable.add_variable("v2", var->value[1]);
		var->eva->SymbolTable.add_variable("v3", var->value[2]);
		var->eva->SymbolTable.add_variable("v4", var->value[3]);
		var->eva->SymbolTable.add_variable("v5", var->value[4]);
		var->eva->SymbolTable.add_variable("v6", var->value[5]);
		var->eva->SymbolTable.add_variable("v7", var->value[6]);
		var->eva->SymbolTable.add_variable("v8", var->value[7]);
		var->eva->SymbolTable.add_variable("v9", var->value[8]);
		var->eva->compileFormula();
		mat3.push_back(var);
	}
	else if (strcmp(var_type, "mat4") == 0)	// MAT4 detected
	{
		varMat4 *var = new varMat4();
		strcpy(var->name, var_name);
		var->loc = my_shader->getUniformLocation(var->name);
		var->eva = new mathDriver(this->my_section);
		var->eva->expression = var_value;
		var->eva->SymbolTable.add_variable("v1", var->value[0]);
		var->eva->SymbolTable.add_variable("v2", var->value[1]);
		var->eva->SymbolTable.add_variable("v3", var->value[2]);
		var->eva->SymbolTable.add_variable("v4", var->value[3]);
		var->eva->SymbolTable.add_variable("v5", var->value[4]);
		var->eva->SymbolTable.add_variable("v6", var->value[5]);
		var->eva->SymbolTable.add_variable("v7", var->value[6]);
		var->eva->SymbolTable.add_variable("v8", var->value[7]);
		var->eva->SymbolTable.add_variable("v9", var->value[8]);
		var->eva->SymbolTable.add_variable("v10", var->value[9]);
		var->eva->SymbolTable.add_variable("v11", var->value[10]);
		var->eva->SymbolTable.add_variable("v12", var->value[11]);
		var->eva->SymbolTable.add_variable("v13", var->value[12]);
		var->eva->SymbolTable.add_variable("v14", var->value[13]);
		var->eva->SymbolTable.add_variable("v15", var->value[14]);
		var->eva->SymbolTable.add_variable("v16", var->value[15]);
		var->eva->compileFormula();
		mat4.push_back(var);
	}
	else if (strcmp(var_type, "sampler2D") == 0)	// Texture (sampler2D) detected
	{
		varSampler2D *var = new varSampler2D();
		strcpy(var->name, var_name);
		var->loc = my_shader->getUniformLocation(var->name);
		// If sampler2D is a fbo...
		if (0 == strncmp("fbo", var_value, 3)) {
			int fbonum;
			sscanf(var_value, "fbo%d", &fbonum);
			if (fbonum<0 || fbonum>(FBO_BUFFERS - 1)) {
				LOG->Error("Section %s: sampler2D fbo not correct, it should be 'fboX', where X=>0 and X<=%d, you choose: %s", this->my_section->identifier.c_str(), (FBO_BUFFERS - 1), var_value);
				return false;
			}
			else {
				var->texture = fbonum;
				var->texGLid = DEMO->fboManager.getOpenGLTextureID(fbonum);
			}
		}
		// Is it s a normal texture...
		else {
			var->texture = DEMO->textureManager.addTexture(s_demo + var_value);
			var->texGLid = DEMO->textureManager.getOpenGLTextureID(var->texture);
		}
		sampler2D.push_back(var);
	}
	return true;
}

