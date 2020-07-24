#include "main.h"
#include "core/shadervars.h"

typedef struct {
	char		clearScreen;	// Clear Screen buffer
	char		clearDepth;		// Clear Depth buffer
	Shader*		shader;			// Shader to apply
	ShaderVars	*shaderVars;	// Shader variables
} drawQuad_section;

static drawQuad_section *local;

// ******************************************************************

sDrawQuad::sDrawQuad() {
	type = SectionType::DrawQuad;
}


bool sDrawQuad::load() {
	// script validation
	if ((this->param.size()) != 2 || (this->strings.size() != 1)) {
		LOG->Error("DrawQuad [%s]: 2 params are needed (Clear the screen buffer & clear depth buffer), and the shader file", this->identifier.c_str());
		return false;
	}
	
	local = (drawQuad_section*) malloc(sizeof(drawQuad_section));
	this->vars = (void *)local;

	// Load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	
	// Load shader
	local->shader = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0]);
	if (!local->shader)
		return false;

	// Create Shader variables
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

void sDrawQuad::init() {
	
}

void sDrawQuad::exec() {
	local = (drawQuad_section*)this->vars;

	local->shader->use();
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	// Set new shader variables values
	local->shaderVars->setValues();

	// Render scene
	EvalBlendingStart();

	glDisable(GL_DEPTH_TEST);
	{
		RES->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawQuad::end() {
	
}

std::string sDrawQuad::debug() {
	return "[ drawQuad id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
