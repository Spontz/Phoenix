#include "main.h"
#include "core/shadervars.h"

struct sDrawQuad : public Section {
public:
	sDrawQuad();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	char		clearScreen;	// Clear Screen buffer
	char		clearDepth;		// Clear Depth buffer
	Shader*		shader;			// Shader to apply
	ShaderVars	*shaderVars;	// Shader variables
};

// ******************************************************************

Section* instance_drawQuad() {
	return new sDrawQuad();
}

sDrawQuad::sDrawQuad() {
	type = SectionType::DrawQuad;
}


bool sDrawQuad::load() {
	// script validation
	if ((param.size()) != 2 || (strings.size() != 1)) {
		LOG->Error("DrawQuad [%s]: 2 params are needed (Clear the screen buffer & clear depth buffer), and the shader file", identifier.c_str());
		return false;
	}

	// Load parameters
	clearScreen = (int)param[0];
	clearDepth = (int)param[1];
	
	// Load shader
	shader = DEMO->shaderManager.addShader(DEMO->dataFolder + strings[0]);
	if (!shader)
		return false;

	// Create Shader variables
	shader->use();
	shaderVars = new ShaderVars(this, shader);

	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		shaderVars->ReadString(uniform[i].c_str());
	}

	// Set shader variables values
	shaderVars->setValues();
	
	return true;
}

void sDrawQuad::init() {
	
}

void sDrawQuad::exec() {
	shader->use();
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (clearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	// Set new shader variables values
	shaderVars->setValues();

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
	return "[ drawQuad id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
}
