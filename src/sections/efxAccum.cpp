#include "main.h"
#include "core/shadervars.h"

typedef struct {
	unsigned int	FboNum;			// Fbo to use (must have 2 color attachments!)
	float			sourceInfluence;// Source influence (0 to 1)
	float			accumInfluence;	// Accumulation influence (0 to 1)
	bool			accumBuffer;	// Accum buffer to use (0 or 1)
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	int				shaderAccum;	// Accumulation Shader to apply
	mathDriver		*exprAccum;		// Equations for the Accum effect
	ShaderVars		*shaderVars;	// Shader variables

} efxAccum_section;

static efxAccum_section *local;

// ******************************************************************

sEfxAccum::sEfxAccum() {
	type = SectionType::EfxAccum;
}


bool sEfxAccum::load() {
	// script validation
	if ((this->param.size()) != 3 || (this->strings.size() < 2)) {
		LOG->Error("EfxAccum [%s]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 2 strings (accum shader files)", this->identifier.c_str());
		return false;
	}
	

	local = (efxAccum_section*) malloc(sizeof(efxAccum_section));
	this->vars = (void *)local;

	// Load parameters
	local->clearScreen = (int)this->param[0];
	local->clearDepth = (int)this->param[1];
	local->FboNum = (int)this->param[2];
	local->accumBuffer = 0;
	
	// Check if the fbo can be used for the effect
	if (local->FboNum < 0 || local->FboNum >= DEMO->fboManager.fbo.size()) {
		LOG->Error("EfxBlur [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", this->identifier.c_str(), local->FboNum, DEMO->fboManager.fbo.size()-1);
		return false;
	}
	
	// Load the Blur amount formula
	local->exprAccum = new mathDriver(this);
	// Load positions, process constants and compile expression
	for (int i = 2; i < strings.size(); i++)
		local->exprAccum->expression += this->strings[i];
	local->exprAccum->SymbolTable.add_variable("SourceInfluence", local->sourceInfluence);
	local->exprAccum->SymbolTable.add_variable("AccumInfluence", local->accumInfluence);
	local->exprAccum->Expression.register_symbol_table(local->exprAccum->SymbolTable);
	if (!local->exprAccum->compileFormula())
		return false;

	// Load Blur shader
	local->shaderAccum = DEMO->shaderManager.addShader(DEMO->dataFolder + this->strings[0], DEMO->dataFolder + this->strings[1]);
	if (local->shaderAccum < 0)
		return false;

	// Create Shader variables
	Shader *my_shaderAccum;
	my_shaderAccum = DEMO->shaderManager.shader[local->shaderAccum];

	// Configure Blur shader
	my_shaderAccum->use();
	my_shaderAccum->setValue("sourceImage", 0);	// The source image will be in the texture unit 0
	my_shaderAccum->setValue("accumImage", 1);	// The accumulated image will be in the texture unit 1

	local->shaderVars = new ShaderVars(this, my_shaderAccum);
	// Read the shader variables
	for (int i = 0; i < this->uniform.size(); i++) {
		local->shaderVars->ReadString(this->uniform[i].c_str());
	}
	// Set shader variables values
	local->shaderVars->setValues();
	
	return true;
}

void sEfxAccum::init() {
	
}

static float lastTime = 0;
static bool firstIteration = true;

void sEfxAccum::exec() {
	local = (efxAccum_section*)this->vars;

	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (local->clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (local->clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Calculate deltaTime
	float deltaTime = this->runTime - lastTime;
	lastTime = this->runTime;

	// Get the shaders
	Shader *my_shaderAccum = DEMO->shaderManager.shader[local->shaderAccum];

	// Evaluate the expression
	local->exprAccum->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		
		{
			// We want to capture the frame in the "Accum Fbo", so first we use the previous fbo for storing the entire image
			DEMO->efxAccumFbo.bind(local->accumBuffer, false, false);
		
			float fps = 1.0f / 60.0f;
			my_shaderAccum->use();
			my_shaderAccum->setValue("sourceInfluence", local->sourceInfluence * (deltaTime/fps) );
			my_shaderAccum->setValue("accumInfluence", 1-(local->accumInfluence * (deltaTime/fps)) );
			local->shaderVars->setValues();

			// Set the screen fbo in texture unit 0
			glActiveTexture(GL_TEXTURE0);//DEMO->fboManager.active(0);
			DEMO->fboManager.bind_tex(local->FboNum);
			
			// Set the accumulation fbo in texture unit 1
			glActiveTexture(GL_TEXTURE1);
			if (firstIteration)
				firstIteration = false;
			local->accumBuffer = !local->accumBuffer; 
			DEMO->efxAccumFbo.bind_tex(local->accumBuffer);

			// Render a quad using the Accum shader (combining the 2 Images)
			RES->Draw_QuadFS();

			DEMO->efxAccumFbo.unbind(false, false); // Unbind drawing into the "Accum Fbo"

			// Adjust back the current fbo
			DEMO->fboManager.bindCurrent();
		}
		

		// Second step: Draw the accum buffer
		Shader* my_shad = DEMO->shaderManager.shader[RES->shdr_QuadTex];

		my_shad->use();
		my_shad->setValue("screenTexture", 0);
		glActiveTexture(GL_TEXTURE0);
		if (firstIteration)
			DEMO->fboManager.bind_tex(local->FboNum);
		else
			DEMO->efxAccumFbo.bind_tex(!local->accumBuffer);
		RES->Draw_QuadFS();

	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxAccum::end() {
	
}

string sEfxAccum::debug() {
	return "[ efxAccum id: " + this->identifier + " layer:" + std::to_string(this->layer) + " ]\n";
}
