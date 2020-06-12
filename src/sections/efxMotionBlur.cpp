#include "main.h"
#include "core/shadervars.h"

typedef struct {
	unsigned int	FboNum;				// Fbo to use (must have 2 color attachments!)
	unsigned int	FPSScale;			// Scale FPS's
	GLuint			bufferColor;		// Attcahment 0 of our FBO
	GLuint			bufferVelocity;		// Attachment 1 of our FBO
	int				shaderMotionBlur;	// Motionblur Shader to apply

	ShaderVars		*shaderVars;	// Shader variables
} efxMotionBlur_section;

static efxMotionBlur_section *local;


sEfxMotionBlur::sEfxMotionBlur() {
	type = SectionType::EfxMotionBlur;
}

bool sEfxMotionBlur::load() {
	local = (efxMotionBlur_section*)malloc(sizeof(efxMotionBlur_section));
	this->vars = (void *)local;
	
	if ((this->param.size()) != 2 || (this->strings.size() != 2)) {
		LOG->Error("EfxMotionBlur [%s]: 2 params are needed (Fbo to use and FPS Scale), and 1 shader files (for Motionblur)", this->identifier.c_str());
		return false;
	}

	local->FboNum = (int)this->param[0];
	local->FPSScale = (int)this->param[1];

	if (local->FPSScale == 0)
		local->FPSScale = 1;

	local->shaderMotionBlur = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	if (local->shaderMotionBlur<0)
		return false;

	Shader *my_shaderMotionBlur;
	my_shaderMotionBlur = DEMO->shaderManager.shader[local->shaderMotionBlur];

	// Configure Blur shader
	my_shaderMotionBlur->use();

	local->shaderVars = new ShaderVars(this, my_shaderMotionBlur);
	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}
	// Set shader variables values
	local->shaderVars->setValues();

	my_shaderMotionBlur->setValue("scene", 0);		// The scene is in the Tex unit 0
	my_shaderMotionBlur->setValue("velocity", 1);	// The velocity is in the Tex unit 1

	// Store the buffers of our FBO (we assume that in Attachment 0 we have the color and in Attachment 1 we have the brights)
	local->bufferColor = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[0];
	local->bufferVelocity = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[1];

	return true;
}

void sEfxMotionBlur::init() {
}


void sEfxMotionBlur::exec() {
	local = (efxMotionBlur_section *)this->vars;

	// Get the shader
	Shader *my_shaderMotionBlur = DEMO->shaderManager.shader[local->shaderMotionBlur];

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		my_shaderMotionBlur->use();

		// Set new shader variables values
		my_shaderMotionBlur->setValue("uVelocityScale", DEMO->fps/local->FPSScale); //uVelocityScale = currentFps / targetFps;
		local->shaderVars->setValues();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, local->bufferColor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, local->bufferVelocity);
		RES->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sEfxMotionBlur::end() {
}

std::string sEfxMotionBlur::debug() {
	local = (efxMotionBlur_section*)this->vars;

	std::string msg;
	msg = "[ efxMotionBlur id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " fbo: " + std::to_string(local->FboNum) + " fps Scale: " + std::to_string(local->FPSScale) + "\n";
	return msg;
}
