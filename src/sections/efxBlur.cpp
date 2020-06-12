#include "main.h"
#include "core/shadervars.h"

typedef struct {
	unsigned int	FboNum;			// Fbo to use (must have 2 color attachments!)
	float			blurAmount;		// Blur layers to apply
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	int				shaderBlur;		// Blur Shader to apply
	mathDriver		*exprBlur;		// Equations for the Blur effect
	ShaderVars		*shaderVars;	// Shader variables

} efxBlur_section;

static efxBlur_section *local;

// ******************************************************************

sEfxBlur::sEfxBlur() {
	type = SectionType::EfxBlur;
}


bool sEfxBlur::load() {
	// script validation
	if ((this->param.size()) != 3 || (this->strings.size() != 3)) {
		LOG->Error("EfxBlur [%s]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 3 strings (One with the formula of the Blur Amount + the 2 blur shader files)", this->identifier.c_str());
		return false;
	}
	

	local = (efxBlur_section*) malloc(sizeof(efxBlur_section));
	this->vars = (void *)local;

	// Load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	local->FboNum = (int)this->param[2];
	
	// Check if the fbo can be used for the effect
	if (local->FboNum < 0 || local->FboNum >= DEMO->fboManager.fbo.size()) {
		LOG->Error("EfxBlur [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", this->identifier.c_str(), local->FboNum, DEMO->fboManager.fbo.size()-1);
		return false;
	}
	
	// Load the Blur amount formula
	local->exprBlur = new mathDriver(this);
	// Load positions, process constants and compile expression
	local->exprBlur->expression = this->strings[0]; // The first string should contain the blur amount
	local->exprBlur->SymbolTable.add_variable("blurAmount", local->blurAmount);
	local->exprBlur->Expression.register_symbol_table(local->exprBlur->SymbolTable);
	if (!local->exprBlur->compileFormula())
		return false;

	// Load Blur shader
	local->shaderBlur = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[1], DEMO->dataFolder + this->strings[2]);
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
	local->shaderVars->setValues();
	
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

	// Evaluate the expression
	local->exprBlur->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// First step: Blur the image from the "fbo attachment 0", and store it in our efxBloom fbo manager (efxBloomFbo)
		bool horizontal = true;
		bool first_iteration = true;
		my_shaderBlur->use();
		DEMO->efxBloomFbo.active(0);

		// Prevent negative Blurs
		if (local->blurAmount < 0)
			local->blurAmount = 0;
		unsigned int iBlurAmount = static_cast<unsigned int>(local->blurAmount);
		for (unsigned int i = 0; i < iBlurAmount; i++)
		{
			my_shaderBlur->setValue("horizontal", horizontal);
			
			// We always draw the First pass in the efxBloom FBO
			DEMO->efxBloomFbo.bind(horizontal, false, false);
			
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

		DEMO->efxBloomFbo.unbind(false, false); // Unbind drawing into an Fbo
		
		// Adjust back the current fbo
		DEMO->fboManager.bindCurrent();
		// Second step: Draw the Blurred image
		RES->Draw_QuadEfxFBOFS(!horizontal);
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxBlur::end() {
	
}

string sEfxBlur::debug() {
	local = (efxBlur_section*)this->vars;

	string msg;
	msg = "[ efxBlur id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
	msg += " fbo: " + std::to_string(local->FboNum) + " Blur Amount: " + std::to_string(local->blurAmount) + "\n";
	return msg;
}
