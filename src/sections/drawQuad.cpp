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
	bool		m_bClearScreen	= true;		// Clear Screen buffer
	bool		m_bClearDepth	= true;		// Clear Depth buffer
	Shader		*m_pShader		= nullptr;	// Shader to apply
	ShaderVars	*m_pVars		= nullptr;	// Shader variables
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
		Logger::error("DrawQuad [%s]: 2 params are needed (Clear the screen buffer & clear depth buffer), and the shader file", identifier.c_str());
		return false;
	}

	// Load parameters
	m_bClearScreen = static_cast<bool>(param[0]);
	m_bClearDepth = static_cast<bool>(param[1]);
	
	// Load shader
	m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
	if (!m_pShader)
		return false;

	// Create Shader variables
	m_pShader->use();
	m_pVars = new ShaderVars(this, m_pShader);

	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		m_pVars->ReadString(uniform[i].c_str());
	}

	// Set shader variables values
	m_pVars->setValues();
	
	return true;
}

void sDrawQuad::init() {
	
}

void sDrawQuad::exec() {
	m_pShader->use();
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (m_bClearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (m_bClearDepth) glClear(GL_DEPTH_BUFFER_BIT);
	
	// Set new shader variables values
	m_pVars->setValues();

	// Render scene
	EvalBlendingStart();

	glDisable(GL_DEPTH_TEST);
	{
		m_demo.m_pRes->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
}

void sDrawQuad::end() {
	
}

std::string sDrawQuad::debug() {
	std::stringstream ss;
	ss << "+ DrawQuad id: " << identifier << " layer: " << layer << std::endl;
	return ss.str();
}
