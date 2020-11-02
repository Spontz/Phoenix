#include "main.h"
#include "core/shadervars.h"

struct sEfxMotionBlur : public Section {
public:
	sEfxMotionBlur();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	unsigned int	m_uiFboNum			= 0;		// Fbo to use (must have 2 color attachments!)
	unsigned int	m_uiFPSScale		= 0;		// Scale FPS's
	GLuint			m_uiBufferColor		= 0;		// Attcahment 0 of our FBO
	GLuint			m_uiBufferVelocity	= 0;		// Attachment 1 of our FBO
	Shader			*m_pShader			= nullptr;	// Motionblur Shader to apply
	ShaderVars		*m_pVars			= nullptr;	// Shader variables
};

// ******************************************************************

Section* instance_efxMotionBlur() {
	return new sEfxMotionBlur();
}

sEfxMotionBlur::sEfxMotionBlur() {
	type = SectionType::EfxMotionBlur;
}

bool sEfxMotionBlur::load() {
	if ((param.size()) != 2 || (strings.size() != 1)) {
		LOG->Error("EfxMotionBlur [%s]: 2 params are needed (Fbo to use and FPS Scale), and 1 shader file (for Motionblur)", identifier.c_str());
		return false;
	}

	m_uiFboNum = static_cast<unsigned int>(param[0]);
	m_uiFPSScale = static_cast<unsigned int>(param[1]);

	if (m_uiFPSScale == 0)
		m_uiFPSScale = 1;

	m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
	if (!m_pShader)
		return false;

	// Configure Blur shader
	m_pShader->use();

	m_pVars = new ShaderVars(this, m_pShader);
	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		m_pVars->ReadString(uniform[i].c_str());
	}
	// Set shader variables values
	m_pVars->setValues();

	m_pShader->setValue("scene", 0);		// The scene is in the Tex unit 0
	m_pShader->setValue("velocity", 1);		// The velocity is in the Tex unit 1

	// Store the buffers of our FBO (we assume that in Attachment 0 we have the color and in Attachment 1 we have the brights)
	m_uiBufferColor = m_demo.m_fboManager.fbo[m_uiFboNum]->m_colorAttachment[0];
	m_uiBufferVelocity = m_demo.m_fboManager.fbo[m_uiFboNum]->m_colorAttachment[1];

	return true;
}

void sEfxMotionBlur::init() {
}


void sEfxMotionBlur::exec() {
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_pShader->use();

		// Set new shader variables values
		m_pShader->setValue("uVelocityScale", m_demo.fps/m_uiFPSScale); //uVelocityScale = currentFps / targetFps;
		m_pVars->setValues();

		glBindTextureUnit(0, m_uiBufferColor);
		glBindTextureUnit(1, m_uiBufferVelocity);
		m_demo.m_pRes->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sEfxMotionBlur::end() {
}

std::string sEfxMotionBlur::debug() {
	std::stringstream ss;
	ss << "+ EfxMotionBlur id: " << identifier << " layer: " << layer << std::endl;
	ss << "  fbo: " << m_uiFboNum << " fps Scale: " << m_uiFPSScale << std::endl;
	return ss.str();
}
