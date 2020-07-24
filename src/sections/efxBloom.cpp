#include "main.h"
#include "core/shadervars.h"

typedef struct {
	unsigned int	FboNum;			// Fbo to use (must have 2 color attachments!)
	float			blurAmount;		// Blur layers to apply
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	Shader*			shaderBlur;		// Blur Shader to apply
	Shader*			shaderBloom;	// Bloom Shader to apply
	mathDriver		*exprBloom;		// Equations for the Bloom effect
	ShaderVars		*shaderVars;	// Shader variables

} efxBloom_section;

static efxBloom_section *local;

// ******************************************************************

sEfxBloom::sEfxBloom() {
	type = SectionType::EfxBloom;
}


bool sEfxBloom::load() {
	// script validation
	if ((this->param.size()) != 3 || (this->strings.size() != 3)) {
		LOG->Error("EfxBloom [%s]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 3 strings (One with the formula of the Blur Amount + 2 with the Blur and Bloom shaders)", this->identifier.c_str());
		return false;
	}
	

	local = (efxBloom_section*) malloc(sizeof(efxBloom_section));
	this->vars = (void *)local;

	// Load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	local->FboNum = (int)this->param[2];
	
	// Check if the fbo can be used for the effect
	if (local->FboNum < 0 || local->FboNum >= DEMO->fboManager.fbo.size()) {
		LOG->Error("EfxBloom [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", this->identifier.c_str(), local->FboNum, DEMO->fboManager.fbo.size()-1);
		return false;
	}
	if (DEMO->fboManager.fbo[local->FboNum]->numAttachments < 2) {
		LOG->Error("EfxBloom [%s]: The fbo specified [%d] has less than 2 attachments, so it cannot be used for bloom effect: Attahment 0 is the color image and Attachment 1 is the brights image", this->identifier.c_str(), local->FboNum);
		return false;
	}
	
	// Load the Blur amount formula
	local->exprBloom = new mathDriver(this);
	// Load positions, process constants and compile expression
	local->exprBloom->expression = this->strings[0]; // The first string should contain the blur amount
	local->exprBloom->SymbolTable.add_variable("blurAmount", local->blurAmount);
	local->exprBloom->Expression.register_symbol_table(local->exprBloom->SymbolTable);
	if (!local->exprBloom->compileFormula())
		return false;

	// Load Blur shader
	local->shaderBlur = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1]);
	// Load Bloom shader
	local->shaderBloom = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[2]);
	if (!local->shaderBlur || !local->shaderBloom)
		return false;

	// Create Shader variables
	
	// Configure Blur shader
	local->shaderBlur->use();
	local->shaderBlur->setValue("image", 0);	// The image is in the texture unit 0

	// Configure Bloom shader (variables are for this shader)
	local->shaderBloom->use();
	local->shaderVars = new ShaderVars(this, local->shaderBloom);
	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}
	// Set shader variables values
	local->shaderVars->setValues();
	local->shaderBloom->setValue("scene", 0);		// The scene is in the Tex unit 0
	local->shaderBloom->setValue("bloomBlur", 1);	// The bloom blur is in the Tex unit 1

	return true;
}

void sEfxBloom::init() {
	
}

void sEfxBloom::exec() {
	local = (efxBloom_section*)this->vars;

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Evaluate the expression
	local->exprBloom->Expression.value();


	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// First step: Blur the image from the "fbo attachment 1", and store it in our efxBloom fbo manager (efxBloomFbo)
		bool horizontal = true;
		bool first_iteration = true;
		local->shaderBlur->use();

		// Prevent negative Blurs
		if (local->blurAmount < 0)
			local->blurAmount = 0;
		unsigned int iBlurAmount = static_cast<unsigned int>(local->blurAmount);
		for (unsigned int i = 0; i < iBlurAmount; i++)
		{
			local->shaderBlur->setValue("horizontal", horizontal);
			
			// We always draw the First pass in the efxBloom FBO
			DEMO->efxBloomFbo.bind(horizontal, false, false);
			
			// If it's the first iteration, we pick the second attachment of our fbo
			// if not, we pick the fbo of our efxBloom
			if (first_iteration)
				DEMO->fboManager.bind_tex(local->FboNum, 0, 1); //Use the second attachment of the fbo
			else
				DEMO->efxBloomFbo.bind_tex(!horizontal, 0, 0);	// Use the texture from our efxBloom
			
			// Render scene
			RES->Draw_QuadFS();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		DEMO->efxBloomFbo.unbind(false, false); // Unbind drawing into an Fbo

		// Second step: Merge the blurred image with the color image (fbo attachment 0)
		local->shaderBloom->use();
		// Set new shader variables values
		local->shaderVars->setValues();

		// Tex unit 0: scene
		DEMO->fboManager.fbo[local->FboNum]->bind_tex(0);
		// Tex unit 1: Bloom blur
		auto tmp = DEMO->efxBloomFbo.fbo;
		DEMO->efxBloomFbo.fbo[!horizontal]->bind_tex(1);

		// Adjust back the current fbo
		DEMO->fboManager.bindCurrent();
		RES->Draw_QuadFS();
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxBloom::end() {
	
}

std::string sEfxBloom::debug() {
	local = (efxBloom_section*)this->vars;

	std::string msg;
	msg = "[ efxBloom id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " fbo: " + std::to_string(local->FboNum) + " Blur Amount: " + std::to_string(local->blurAmount) + "\n";
	return msg;
}
