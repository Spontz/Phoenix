#include "main.h"
#include "core/shadervars.h"

typedef struct {
	int				shader;
	ShaderVars		*vars;
} glslshaderbind_section;

static glslshaderbind_section *local;

// ******************************************************************

sGLSLShaderBind::sGLSLShaderBind() {
	type = SectionType::GLSLShaderBind;
}

bool sGLSLShaderBind::load() {
	
	string s_demo = DEMO->demoDir;
	Shader*			my_shader;

	// script validation
	// 2 strings needed: Vertex and fragment shader path
	if (this->stringNum < 2) {
		LOG->Error("GLSLShaderBind [%s]: 2 strings are needed: vertex and fragment shader files", this->identifier.c_str());
		return false;
	}

	local = (glslshaderbind_section*) malloc(sizeof(glslshaderbind_section));
	this->vars = (void *)local;

	// load Shader
	local->shader = DEMO->shaderManager.addShader(s_demo + this->strings[0], s_demo + this->strings[1]);
	if (local->shader == -1)
		return false;
	
	my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();

	// Create variables
	local->vars = new ShaderVars(this, my_shader);
	
	// Read the variables
	for (int i = 2; i < this->stringNum; i++) {
		local->vars->ReadString(this->strings[i].c_str());
	}

	// Set the values
	local->vars->setValues(true);

	// Unbind any shader used
	DEMO->shaderManager.unbindShaders();
	return true;
}

void sGLSLShaderBind::init() {
	
}

void sGLSLShaderBind::exec() {
	
	local = (glslshaderbind_section*)this->vars;
	if (local->shader == -1)
		return;

	// Choose the proper program shader
	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	
	// Reset the shader values
	local->vars->setValues(false);

	RES->Draw_QuadFBOFS_noShader(0);
}

void sGLSLShaderBind::end() {
	
}
