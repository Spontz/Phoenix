#include "main.h"
#include "core/shadervars.h"

typedef struct {
	int				shaderFader;	// Fader Shader to apply
	ShaderVars		*shaderVars;	// Shader variables
} efxFader_section;

static efxFader_section *local;

// ******************************************************************

sEfxFader::sEfxFader() {
	type = SectionType::EfxFader;
}


bool sEfxFader::load() {
	// script validation
	if ((this->param.size()) != 0 || (this->strings.size() != 2)) {
		LOG->Error("EfxFader [%s]: 2 shader files required", this->identifier.c_str());
		return false;
	}
	

	local = (efxFader_section*) malloc(sizeof(efxFader_section));
	this->vars = (void *)local;

	// Load Fader shader
	local->shaderFader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	if (local->shaderFader < 0)
		return false;

	// Create shader variables
	Shader *my_shaderFader;
	my_shaderFader = DEMO->shaderManager.shader[local->shaderFader];

	// Configure Fader shader
	my_shaderFader->use();
	local->shaderVars = new ShaderVars(this, my_shaderFader);
	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}
	
	// Set shader variables values
	local->shaderVars->setValues();

	return true;
}

void sEfxFader::init() {
	
}

void sEfxFader::exec() {
	local = (efxFader_section*)this->vars;

	// Get the shaders
	Shader *my_shaderFader = DEMO->shaderManager.shader[local->shaderFader];

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		my_shaderFader->use();
		// Set shader variables values
		local->shaderVars->setValues();
		// Render scene
		RES->Draw_QuadFS();
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxFader::end() {
	
}

string sEfxFader::debug() {
	return string();
}
