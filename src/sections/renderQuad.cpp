#include "main.h"
#include "core/shadervars.h"

typedef struct {
	char		clearScreen;	// Clear Screen buffer
	char		clearDepth;		// Clear Depth buffer
	int			shader;			// Shader to apply
	ShaderVars	*shaderVars;	// Shader variables
} renderQuad_section;

static renderQuad_section *local;

// ******************************************************************

sRenderQuad::sRenderQuad() {
	type = SectionType::RenderQuad;
}


bool sRenderQuad::load() {
	// script validation
	if ((this->param.size()) != 2 || (this->strings.size() != 2)) {
		LOG->Error("RenderQuad [%s]: 2 params are needed (Clear the screen buffer & clear depth buffer), and 2 shader files (vertex and fragment)", this->identifier.c_str());
		return false;
	}
	
	local = (renderQuad_section*) malloc(sizeof(renderQuad_section));
	this->vars = (void *)local;

	// Load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	
	// Load shader
	string s_demo = DEMO->demoDir;
	local->shader = DEMO->shaderManager.addShader(s_demo + this->strings[0], s_demo + this->strings[1]);
	if (local->shader < 0)
		return false;

	// Create Shader variables
	Shader *my_shader;
	my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	local->shaderVars = new ShaderVars(this, my_shader);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	local->shaderVars->setValues(true);
	
	return true;
}

void sRenderQuad::init() {
	
}

void sRenderQuad::exec() {
	local = (renderQuad_section*)this->vars;

	Shader *my_shader = DEMO->shaderManager.shader[local->shader];
	my_shader->use();
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	// Set new shader variables values
	local->shaderVars->setValues(false);

	// Render scene
	EvalBlendingStart();

	glDisable(GL_DEPTH_TEST);
	{
		RES->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sRenderQuad::end() {
	
}
