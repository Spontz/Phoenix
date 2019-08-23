#include "main.h"
#include "core/shadervars.h"

typedef struct {
	unsigned int	FboNum;			// Fbo to use (must have 2 color attachments!)
	unsigned int	blurAmount;		// Blur layers to apply
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	int				shaderBlur;		// Blur Shader to apply
	ShaderVars		*shaderVars;	// Shader variables

} efxBlur_section;

static efxBlur_section *local;

// ******************************************************************

sEfxBlur::sEfxBlur() {
	type = SectionType::EfxBlur;
}


bool sEfxBlur::load() {
	// script validation
	if ((this->param.size()) != 4 || (this->strings.size() != 2)) {
		LOG->Error("EfxBlur [%s]: 4 params are needed (Clear the screen & depth buffers, Fbo to use and Blur Amount), and 2 blur shader files", this->identifier.c_str());
		return false;
	}
	

	local = (efxBlur_section*) malloc(sizeof(efxBlur_section));
	this->vars = (void *)local;

	// Load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	local->FboNum = (int)this->param[2];
	local->blurAmount = (unsigned int)this->param[3];

	// Check if the fbo can be used for the effect
	if (local->FboNum < 0 || local->FboNum >= DEMO->fboManager.fbo.size()) {
		LOG->Error("EfxBlur [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", this->identifier.c_str(), local->FboNum, DEMO->fboManager.fbo.size()-1);
		return false;
	}
	
	// Load Blur shader
	local->shaderBlur = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	if (local->shaderBlur < 0)
		return false;

	// Create Shader variables
	Shader *my_shaderBlur;
	my_shaderBlur = DEMO->shaderManager.shader[local->shaderBlur];

	// Configure Blur shader
	my_shaderBlur->use();
	my_shaderBlur->setValue("image", 0);	// The image is in the texture unit 0

	local->shaderVars = new ShaderVars(this, my_shaderBlur);
	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}
	// Set shader variables values
	local->shaderVars->setValues(true);
	
	return true;
}

void sEfxBlur::init() {
	
}

void sEfxBlur::exec() {
	local = (efxBlur_section*)this->vars;

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Get the shaders
	Shader *my_shaderBlur = DEMO->shaderManager.shader[local->shaderBlur];

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// First step: Blur the image from the "fbo attachment 0", and store it in our efxBloom fbo manager (efxBloomFbo)
		bool horizontal = true;
		bool first_iteration = true;
		my_shaderBlur->use();
		DEMO->efxBloomFbo.active(0);

		for (unsigned int i = 0; i < local->blurAmount; i++)
		{
			my_shaderBlur->setValue("horizontal", horizontal);
			
			// We always draw the First pass in the efxBloom FBO
			DEMO->efxBloomFbo.bind(horizontal);
			
			// If it's the first iteration, we pick the fbo
			// if not, we pick the fbo of our efxBloom
			if (first_iteration)
				DEMO->fboManager.bind_tex(local->FboNum);
			else
				DEMO->efxBloomFbo.bind_tex(!horizontal);
			
			// Render scene
			RES->Draw_QuadFS();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		DEMO->efxBloomFbo.unbind(); // Unbind drawing into an Fbo

		// Second step: Draw the Blurred image
		RES->Draw_QuadEfxFBOFS(!horizontal);
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxBlur::end() {
	
}
