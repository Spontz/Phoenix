#include "main.h"


typedef struct {
	unsigned int	FboNum;				// Fbo to use (must have 2 color attachments!)
	GLuint			bufferColor;		// Attcahment 0 of our FBO
	GLuint			bufferVelocity;		// Attachment 1 of our FBO
	int				shaderMotionBlur;	// Motionblur Shader to apply
} test_section;

static test_section *local;


sTest::sTest() {
	type = SectionType::Test;
}

bool sTest::load() {
	local = (test_section*)malloc(sizeof(test_section));
	this->vars = (void *)local;
	
	if ((this->param.size()) != 1 || (this->strings.size() != 2)) {
		LOG->Error("EfxMotionBlur [%s]: 1 params are needed (Fbo to use), and 1 shader files (for Motionblur)", this->identifier.c_str());
		return false;
	}

	local->FboNum = (int)this->param[0];

	local->shaderMotionBlur = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	if (local->shaderMotionBlur<0)
		return false;

	Shader *my_shaderMotionBlur;
	my_shaderMotionBlur = DEMO->shaderManager.shader[local->shaderMotionBlur];

	// Configure Blur shader
	my_shaderMotionBlur->use();

	/*
	local->shaderVars = new ShaderVars(this, my_shaderMotionBlur);
	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}
	// Set shader variables values
	local->shaderVars->setValues(true);
	*/
	my_shaderMotionBlur->setValue("scene", 0);		// The scene is in the Tex unit 0
	my_shaderMotionBlur->setValue("velocity", 1);	// The velocity is in the Tex unit 1

	// Store the buffers of our FBO (we assume that in Attachment 0 we have the color and in Attachment 1 we have the brights)
	local->bufferColor = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[0];
	local->bufferVelocity = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[1];

	return true;
}

void sTest::init() {
}


void sTest::exec() {
	local = (test_section *)this->vars;



	// Clear the screen and depth buffers depending of the parameters passed by the user
	//glClear(GL_COLOR_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT);

	// Get the shader
	Shader *my_shaderMotionBlur = DEMO->shaderManager.shader[local->shaderMotionBlur];

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Second step: Merge the velocity image with the color image (fbo attachment 0)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		my_shaderMotionBlur->use();

		my_shaderMotionBlur->setValue("uVelocityScale", DEMO->fps/30.0f); //uVelocityScale = currentFps / targetFps;

		// Set new shader variables values
		//local->shaderVars->setValues(false);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, local->bufferColor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, local->bufferVelocity);
		RES->Draw_QuadFS();


	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sTest::end() {
}
