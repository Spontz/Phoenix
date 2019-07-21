#include "main.h"
#include "core/shadervars.h"

typedef struct {
	unsigned int	FboNum;		// Fbo to use (must have 2 color attachments!)
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	int				shaderBlur;		// Blur Shader to apply
	int				shaderBloom;	// Bloom Shader to apply
	ShaderVars		*shaderVars;	// Shader variables
	// The 2 buffers for the FBO's
	GLuint			pingpongFBO[2];
	GLuint			pingpongColorbuffers[2];
} drawBloom_section;

static drawBloom_section *local;

// ******************************************************************

sDrawBloom::sDrawBloom() {
	type = SectionType::DrawBloom;
}


bool sDrawBloom::load() {
	// script validation
	if ((this->param.size()) != 3 || (this->strings.size() != 4)) {
		LOG->Error("DrawBloom [%s]: 3 params are needed (Fbo to use, Clear the screen buffer & clear depth buffer), and 2 shader files (2 for Blur and 2 for Bloom)", this->identifier.c_str());
		return false;
	}
	
	local = (drawBloom_section*) malloc(sizeof(drawBloom_section));
	this->vars = (void *)local;

	// Load parameters
	local->FboNum = (int)this->param[0];
	local->clearScreen = (int)this->param[1];
	local->clearDepth = (int)this->param[2];

	// Check if the fbo can be used for the effect
	if (local->FboNum < 0 || local->FboNum >= DEMO->fboManager.fbo.size()) {
		LOG->Error("DrawBloom [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", this->identifier.c_str(), local->FboNum, DEMO->fboManager.fbo.size()-1);
		return false;
	}
	if (DEMO->fboManager.fbo[local->FboNum]->numAttachments < 2) {
		LOG->Error("DrawBloom [%s]: The fbo specified [%d] has less than 2 attachments, so it cannot be used for bloom effect: Attahment 0 is the color image and Attachment 1 is the brights image", this->identifier.c_str(), local->FboNum);
		return false;
	}
	
	// Load Blur shader
	local->shaderBlur = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	// Load Bloom shader
	local->shaderBloom = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[2], DEMO->dataFolder + this->strings[3]);
	if (local->shaderBlur < 0 || local->shaderBloom < 0)
		return false;

	// Create Shader variables
	Shader *my_shaderBlur;
	my_shaderBlur = DEMO->shaderManager.shader[local->shaderBlur];
	my_shaderBlur->use();
	local->shaderVars = new ShaderVars(this, my_shaderBlur);

	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}

	// Set shader variables values
	local->shaderVars->setValues(true);

	// TODO: Deleted this HACK
	//local->shaderVars->sampler2D[0]->texGLid = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[1];

	// Create the ping-pong buffers
	// TODO: The ping-pong buffers could be shared in all the engine, there is no need to create new buffers per each effect we set in the engine!
	// TODO: Create the buffers in the Resource class
	glGenFramebuffers(2, local->pingpongFBO);
	glGenTextures(2, local->pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, local->pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, local->pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, GLDRV->width, GLDRV->height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, local->pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LOG->Error("DrawBloom [%s]: The internal framebuffers could not be created!", this->identifier.c_str());
			return false;
		}
	}
	
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
		{
			int horizontal = 1;
			bool first_iteration = true;
			unsigned int amount = 10;
			my_shaderBlur->use();
			// Set new shader variables values
			local->shaderVars->setValues(false);
			

			// TODO: This could be done in the loading
			GLuint bufferID = DEMO->fboManager.fbo[local->FboNum]->colorBufferID[1]; // We assume that in the attachment 1 we have the brights buffer
			for (unsigned int i = 0; i < amount; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, local->pingpongFBO[horizontal]);
				my_shaderBlur->setValue("horizontal", horizontal);
				//int kk = my_shaderBlur->getUniformLocation("image");
				glBindTexture(GL_TEXTURE_2D, first_iteration ? bufferID : local->pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
				// Render scene
				RES->Draw_QuadFS();
				if (horizontal == 1)
					horizontal = 0;
				else
					horizontal = 1;
				//horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Second step: Merge the blurred image with the color image (fbo attachment 0)
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sDrawBloom::end() {
	
}
