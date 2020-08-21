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
	unsigned int	FboNum;				// Fbo to use (must have 2 color attachments!)
	unsigned int	FPSScale;			// Scale FPS's
	GLuint			bufferColor;		// Attcahment 0 of our FBO
	GLuint			bufferVelocity;		// Attachment 1 of our FBO
	Shader*			shader;				// Motionblur Shader to apply

	ShaderVars		*shaderVars;	// Shader variables
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

	FboNum = (int)param[0];
	FPSScale = (int)param[1];

	if (FPSScale == 0)
		FPSScale = 1;

	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[0]);
	if (!shader)
		return false;

	// Configure Blur shader
	shader->use();

	shaderVars = new ShaderVars(this, shader);
	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		shaderVars->ReadString(uniform[i].c_str());
	}
	// Set shader variables values
	shaderVars->setValues();

	shader->setValue("scene", 0);		// The scene is in the Tex unit 0
	shader->setValue("velocity", 1);	// The velocity is in the Tex unit 1

	// Store the buffers of our FBO (we assume that in Attachment 0 we have the color and in Attachment 1 we have the brights)
	bufferColor = m_demo.fboManager.fbo[FboNum]->m_colorAttachment[0];
	bufferVelocity = m_demo.fboManager.fbo[FboNum]->m_colorAttachment[1];

	return true;
}

void sEfxMotionBlur::init() {
}


void sEfxMotionBlur::exec() {
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader->use();

		// Set new shader variables values
		shader->setValue("uVelocityScale", m_demo.fps/FPSScale); //uVelocityScale = currentFps / targetFps;
		shaderVars->setValues();

		glBindTextureUnit(0, bufferColor);
		glBindTextureUnit(1, bufferVelocity);
		m_demo.res->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sEfxMotionBlur::end() {
}

std::string sEfxMotionBlur::debug() {
	std::string msg;
	msg = "[ efxMotionBlur id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " fbo: " + std::to_string(FboNum) + " fps Scale: " + std::to_string(FPSScale) + "\n";
	return msg;
}
