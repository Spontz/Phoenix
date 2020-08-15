#include "main.h"
#include "core/shadervars.h"

struct sEfxAccum : public Section {
public:
	sEfxAccum();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	demokernel& demo = demokernel::GetInstance();
	unsigned int	FboNum;			// Fbo to use (must have 2 color attachments!)
	float			sourceInfluence;// Source influence (0 to 1)
	float			accumInfluence;	// Accumulation influence (0 to 1)
	bool			accumBuffer;	// Accum buffer to use (0 or 1)
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	Shader*			shader;			// Accumulation Shader to apply
	mathDriver		*exprAccum;		// Equations for the Accum effect
	ShaderVars		*shaderVars;	// Shader variables
};

// ******************************************************************

Section* instance_efxAccum() {
	return new sEfxAccum();
}

sEfxAccum::sEfxAccum() {
	type = SectionType::EfxAccum;
}


bool sEfxAccum::load() {
	// script validation
	if ((param.size()) != 3 || (strings.size() < 1)) {
		LOG->Error("EfxAccum [%s]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 1 string (accum shader)", identifier.c_str());
		return false;
	}
	
	// Load parameters
	clearScreen = (int)param[0];
	clearDepth = (int)param[1];
	FboNum = (int)param[2];
	accumBuffer = 0;
	
	// Check if the fbo can be used for the effect
	if (FboNum < 0 || FboNum >= demo.fboManager.fbo.size()) {
		LOG->Error("EfxBlur [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", identifier.c_str(), FboNum, demo.fboManager.fbo.size()-1);
		return false;
	}
	
	// Load the Blur amount formula
	exprAccum = new mathDriver(this);
	// Load positions, process constants and compile expression
	for (int i = 1; i < strings.size(); i++)
		exprAccum->expression += strings[i];
	exprAccum->SymbolTable.add_variable("SourceInfluence", sourceInfluence);
	exprAccum->SymbolTable.add_variable("AccumInfluence", accumInfluence);
	exprAccum->Expression.register_symbol_table(exprAccum->SymbolTable);
	if (!exprAccum->compileFormula())
		return false;

	// Load Blur shader
	shader = demo.shaderManager.addShader(demo.dataFolder + strings[0]);
	if (!shader)
		return false;

	// Configure shader
	shader->use();
	shader->setValue("sourceImage", 0);	// The source image will be in the texture unit 0
	shader->setValue("accumImage", 1);	// The accumulated image will be in the texture unit 1

	shaderVars = new ShaderVars(this, shader);
	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		shaderVars->ReadString(uniform[i].c_str());
	}
	// Set shader variables values
	shaderVars->setValues();
	
	return true;
}

void sEfxAccum::init() {
	
}

static float lastTime = 0;
static bool firstIteration = true;

void sEfxAccum::exec() {
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Calculate deltaTime
	float deltaTime = runTime - lastTime;
	lastTime = runTime;

	// Evaluate the expression
	exprAccum->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		
		{
			// We want to capture the frame in the "Accum Fbo", so first we use the previous fbo for storing the entire image
			demo.efxAccumFbo.bind(accumBuffer, false, false);
		
			float fps = 1.0f / 60.0f;
			shader->use();
			shader->setValue("sourceInfluence", sourceInfluence * (deltaTime/fps) );
			shader->setValue("accumInfluence", 1-(accumInfluence * (deltaTime/fps)) );
			shaderVars->setValues();

			// Set the screen fbo in texture unit 0
			demo.fboManager.bind_tex(FboNum, 0);
			
			// Set the accumulation fbo in texture unit 1
			if (firstIteration)
				firstIteration = false;
			accumBuffer = !accumBuffer; 
			demo.efxAccumFbo.bind_tex(accumBuffer, 1);

			// Render a quad using the Accum shader (combining the 2 Images)
			RES->Draw_QuadFS();

			demo.efxAccumFbo.unbind(false, false); // Unbind drawing into the "Accum Fbo"

			// Adjust back the current fbo
			demo.fboManager.bindCurrent();
		}
		

		// Second step: Draw the accum buffer
		RES->shdr_QuadTex->use();
		RES->shdr_QuadTex->setValue("screenTexture", 0);
		if (firstIteration)
			demo.fboManager.bind_tex(FboNum, 0);
		else
			demo.efxAccumFbo.bind_tex(!accumBuffer, 0);
		RES->Draw_QuadFS();

	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxAccum::end() {
	
}

std::string sEfxAccum::debug() {
	std::string msg;
	msg = "[ efxAccum id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " fbo: " + std::to_string(FboNum) + " Source Influence: " + std::to_string(sourceInfluence) + "\n";
	return msg;
}
