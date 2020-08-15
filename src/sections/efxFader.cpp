#include "main.h"
#include "core/shadervars.h"

struct sEfxFader : public Section {
public:
	sEfxFader();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	Shader*			shader;			// Fader Shader to apply
	ShaderVars		*shaderVars;	// Shader variables
};

// ******************************************************************

Section* instance_efxFader() {
	return new sEfxFader();
}

sEfxFader::sEfxFader() {
	type = SectionType::EfxFader;
}


bool sEfxFader::load() {
	// script validation
	if ((param.size()) != 0 || (strings.size() != 1)) {
		LOG->Error("EfxFader [%s]: 1 shader file required", identifier.c_str());
		return false;
	}
	
	// Load Fader shader
	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[0]);
	if (!shader)
		return false;

	// Configure Fader shader
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

void sEfxFader::init() {
	
}

void sEfxFader::exec() {
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		shader->use();
		// Set shader variables values
		shaderVars->setValues();
		// Render scene
		RES->Draw_QuadFS();
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxFader::end() {
	
}

std::string sEfxFader::debug() {
	return "[ efxFader id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
}
