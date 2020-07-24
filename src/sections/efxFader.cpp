#include "main.h"
#include "core/shadervars.h"

typedef struct {
	Shader*			shader;			// Fader Shader to apply
	ShaderVars		*shaderVars;	// Shader variables
} efxFader_section;

static efxFader_section *local;

// ******************************************************************

sEfxFader::sEfxFader() {
	type = SectionType::EfxFader;
}


bool sEfxFader::load() {
	// script validation
	if ((this->param.size()) != 0 || (this->strings.size() != 1)) {
		LOG->Error("EfxFader [%s]: 1 shader file required", this->identifier.c_str());
		return false;
	}
	

	local = (efxFader_section*) malloc(sizeof(efxFader_section));
	this->vars = (void *)local;

	// Load Fader shader
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0]);
	if (!local->shader)
		return false;

	// Configure Fader shader
	local->shader->use();
	local->shaderVars = new ShaderVars(this, local->shader);
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

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		local->shader->use();
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

std::string sEfxFader::debug() {
	return "[ efxFader id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
