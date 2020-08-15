#include "main.h"
#include "core/shadervars.h"

struct sEfxBloom : public Section {
public:
	sEfxBloom();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	unsigned int	FboNum;			// Fbo to use (must have 2 color attachments!)
	float			blurAmount;		// Blur layers to apply
	char			clearScreen;	// Clear Screen buffer
	char			clearDepth;		// Clear Depth buffer
	Shader*			shaderBlur;		// Blur Shader to apply
	Shader*			shaderBloom;	// Bloom Shader to apply
	mathDriver		*exprBloom;		// Equations for the Bloom effect
	ShaderVars		*shaderVars;	// Shader variables
};

// ******************************************************************

Section* instance_efxBloom() {
	return new sEfxBloom();
}

sEfxBloom::sEfxBloom() {
	type = SectionType::EfxBloom;
}


bool sEfxBloom::load() {
	// script validation
	if ((param.size()) != 3 || (strings.size() != 3)) {
		LOG->Error("EfxBloom [%s]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 3 strings (One with the formula of the Blur Amount + 2 with the Blur and Bloom shaders)", identifier.c_str());
		return false;
	}
	
	// Load parameters
	clearScreen = (int)param[0];
	clearDepth = (int)param[1];
	FboNum = (int)param[2];
	
	// Check if the fbo can be used for the effect
	if (FboNum < 0 || FboNum >= m_demo.fboManager.fbo.size()) {
		LOG->Error("EfxBloom [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", identifier.c_str(), FboNum, m_demo.fboManager.fbo.size()-1);
		return false;
	}
	if (m_demo.fboManager.fbo[FboNum]->numAttachments < 2) {
		LOG->Error("EfxBloom [%s]: The fbo specified [%d] has less than 2 attachments, so it cannot be used for bloom effect: Attahment 0 is the color image and Attachment 1 is the brights image", identifier.c_str(), FboNum);
		return false;
	}
	
	// Load the Blur amount formula
	exprBloom = new mathDriver(this);
	// Load positions, process constants and compile expression
	exprBloom->expression = strings[0]; // The first string should contain the blur amount
	exprBloom->SymbolTable.add_variable("blurAmount", blurAmount);
	exprBloom->Expression.register_symbol_table(exprBloom->SymbolTable);
	if (!exprBloom->compileFormula())
		return false;

	// Load Blur shader
	shaderBlur = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[1]);
	// Load Bloom shader
	shaderBloom = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[2]);
	if (!shaderBlur || !shaderBloom)
		return false;

	// Create Shader variables
	
	// Configure Blur shader
	shaderBlur->use();
	shaderBlur->setValue("image", 0);	// The image is in the texture unit 0

	// Configure Bloom shader (variables are for this shader)
	shaderBloom->use();
	shaderVars = new ShaderVars(this, shaderBloom);
	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		shaderVars->ReadString(uniform[i].c_str());
	}
	// Set shader variables values
	shaderVars->setValues();
	shaderBloom->setValue("scene", 0);		// The scene is in the Tex unit 0
	shaderBloom->setValue("bloomBlur", 1);	// The bloom blur is in the Tex unit 1

	return true;
}

void sEfxBloom::init() {
	
}

void sEfxBloom::exec() {
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Evaluate the expression
	exprBloom->Expression.value();


	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// First step: Blur the image from the "fbo attachment 1", and store it in our efxBloom fbo manager (efxBloomFbo)
		bool horizontal = true;
		bool first_iteration = true;
		shaderBlur->use();

		// Prevent negative Blurs
		if (blurAmount < 0)
			blurAmount = 0;
		unsigned int iBlurAmount = static_cast<unsigned int>(blurAmount);
		for (unsigned int i = 0; i < iBlurAmount; i++)
		{
			shaderBlur->setValue("horizontal", horizontal);
			
			// We always draw the First pass in the efxBloom FBO
			m_demo.efxBloomFbo.bind(horizontal, false, false);
			
			// If it's the first iteration, we pick the second attachment of our fbo
			// if not, we pick the fbo of our efxBloom
			if (first_iteration)
				m_demo.fboManager.bind_tex(FboNum, 0, 1); //Use the second attachment of the fbo
			else
				m_demo.efxBloomFbo.bind_tex(!horizontal, 0, 0);	// Use the texture from our efxBloom
			
			// Render scene
			RES->Draw_QuadFS();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		m_demo.efxBloomFbo.unbind(false, false); // Unbind drawing into an Fbo

		// Second step: Merge the blurred image with the color image (fbo attachment 0)
		shaderBloom->use();
		// Set new shader variables values
		shaderVars->setValues();

		// Tex unit 0: scene
		m_demo.fboManager.fbo[FboNum]->bind_tex(0);
		// Tex unit 1: Bloom blur
		auto tmp = m_demo.efxBloomFbo.fbo;
		m_demo.efxBloomFbo.fbo[!horizontal]->bind_tex(1);

		// Adjust back the current fbo
		m_demo.fboManager.bindCurrent();
		RES->Draw_QuadFS();
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxBloom::end() {
	
}

std::string sEfxBloom::debug() {
	std::string msg;
	msg = "[ efxBloom id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " fbo: " + std::to_string(FboNum) + " Blur Amount: " + std::to_string(blurAmount) + "\n";
	return msg;
}
