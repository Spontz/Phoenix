#include "main.h"
#include "core/shadervars.h"

typedef struct {
	unsigned int	FboNum;		// Fbo to use (must have 2 color attachments!)
	GLuint			bufferColor;	// Attcahment 0 of our FBO
	GLuint			bufferBrights;	// Attachment 1 of our fbo
	unsigned int	blurAmount;		// Blur layers to apply
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	int				shaderBlur;		// Blur Shader to apply
	int				shaderBloom;	// Bloom Shader to apply
	ShaderVars		*shaderVars;	// Shader variables

	// Pointer to the buffers form the Resource manager
	GLuint	*pingpongFBO[2];
	GLuint	*pingpongColorbuffer[2];
} drawBloom_section;

static drawBloom_section *local;

// ******************************************************************

sDrawBloom::sDrawBloom() {
	type = SectionType::DrawBloom;
}


bool sDrawBloom::load() {
	// script validation
	if (RES->bloomLoaded == false) {
		LOG->Error("DrawBloom [%s]: The internal resources could not be created, so Bloom effect will not work... disabling it!", this->identifier.c_str());
		return false;
	}

	if ((this->param.size()) != 4 || (this->strings.size() != 4)) {
		LOG->Error("DrawBloom [%s]: 4 params are needed (Clear the screen & depth buffers, Fbo to use and Blur Amount), and 2 shader files (2 for Blur and 2 for Bloom)", this->identifier.c_str());
		return false;
	}
	

	local = (drawBloom_section*) malloc(sizeof(drawBloom_section));
	this->vars = (void *)local;

	// Load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	local->FboNum = (int)this->param[2];
	local->blurAmount = (unsigned int)this->param[3];

	// Check if the fbo can be used for the effect
	if (local->FboNum < 0 || local->FboNum >= DEMO->fboManager.fbo.size()) {
		LOG->Error("DrawBloom [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", this->identifier.c_str(), local->FboNum, DEMO->fboManager.fbo.size()-1);
		return false;
	}
	if (DEMO->fboManager.fbo[local->FboNum]->numAttachments < 2) {
		LOG->Error("DrawBloom [%s]: The fbo specified [%d] has less than 2 attachments, so it cannot be used for bloom effect: Attahment 0 is the color image and Attachment 1 is the brights image", this->identifier.c_str(), local->FboNum);
		return false;
	}
	
	// Store the buffers of our FBO (we assume that in Attachment 0 we have the color and in Attachment 1 we have the brights)
	local->bufferColor = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[0];
	local->bufferBrights = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[1];


	// Load Blur shader
	local->shaderBlur = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	// Load Bloom shader
	local->shaderBloom = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[2], DEMO->dataFolder + this->strings[3]);
	if (local->shaderBlur < 0 || local->shaderBloom < 0)
		return false;

	// Create Shader variables
	Shader *my_shaderBlur;
	Shader *my_shaderBloom;
	my_shaderBlur = DEMO->shaderManager.shader[local->shaderBlur];
	my_shaderBloom = DEMO->shaderManager.shader[local->shaderBloom];

	// Configure Shader Blur
	my_shaderBlur->use();
	my_shaderBlur->setValue("image", 0);	// The image is in the texture unit 0

	// Configure Shader Bloom (variables are for this shader)
	my_shaderBloom->use();
	local->shaderVars = new ShaderVars(this, my_shaderBloom);
	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}
	// Set shader variables values
	local->shaderVars->setValues(true);
	my_shaderBloom->setValue("scene", 0);		// The scene is in the Tex unit 0
	my_shaderBloom->setValue("bloomBlur", 1);	// The bloom blur is in the Tex unit 1

	// Get the FBO's and Color buffers from the Resources manager
	local->pingpongFBO[0] = &(RES->bloomPingpongFBO[0]);
	local->pingpongFBO[1] = &(RES->bloomPingpongFBO[1]);
	local->pingpongColorbuffer[0] = &(RES->bloomPingpongColorbuffer[0]);
	local->pingpongColorbuffer[1] = &(RES->bloomPingpongColorbuffer[1]);

	return true;
}

void sDrawBloom::init() {
	
}

void sDrawBloom::exec() {
	local = (drawBloom_section*)this->vars;

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Get the shaders
	Shader *my_shaderBlur = DEMO->shaderManager.shader[local->shaderBlur];
	Shader *my_shaderBloom = DEMO->shaderManager.shader[local->shaderBloom];


	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// First step: Blur the image (fbo attachment 1)
		bool horizontal = true;
		bool first_iteration = true;
		my_shaderBlur->use();
		for (unsigned int i = 0; i < local->blurAmount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, *local->pingpongFBO[horizontal]);
			my_shaderBlur->setValue("horizontal", horizontal);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? local->bufferBrights : *local->pingpongColorbuffer[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
			// Render scene
			RES->Draw_QuadFS();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Second step: Merge the blurred image with the color image (fbo attachment 0)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		my_shaderBloom->use();
		// Set new shader variables values
		local->shaderVars->setValues(false);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, local->bufferColor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, *local->pingpongColorbuffer[!horizontal]);
		RES->Draw_QuadFS();
		

	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sDrawBloom::end() {
	
}
