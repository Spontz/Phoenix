#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/shadervars.h"

struct sEfxBlur : public Section {
public:
	sEfxBlur();
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
	Shader*			shader;			// Blur Shader to apply
	mathDriver		*exprBlur;		// Equations for the Blur effect
	ShaderVars		*shaderVars;	// Shader variables
};

// ******************************************************************

Section* instance_efxBlur() {
	return new sEfxBlur();
}

sEfxBlur::sEfxBlur() {
	type = SectionType::EfxBlur;
}


bool sEfxBlur::load() {
	// script validation
	if ((param.size()) != 3 || (strings.size() != 2)) {
		LOG->Error("EfxBlur [%s]: 3 params are needed (Clear the screen & depth buffers and Fbo to use), and 3 strings (One with the formula of the Blur Amount + blur shader file)", identifier.c_str());
		return false;
	}

	// Load parameters
	clearScreen = (int)param[0];
	clearDepth = (int)param[1];
	FboNum = (int)param[2];
	
	// Check if the fbo can be used for the effect
	if (FboNum < 0 || FboNum >= m_demo.fboManager.fbo.size()) {
		LOG->Error("EfxBlur [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", identifier.c_str(), FboNum, m_demo.fboManager.fbo.size()-1);
		return false;
	}
	
	// Load the Blur amount formula
	exprBlur = new mathDriver(this);
	// Load positions, process constants and compile expression
	exprBlur->expression = strings[0]; // The first string should contain the blur amount
	exprBlur->SymbolTable.add_variable("blurAmount", blurAmount);
	exprBlur->Expression.register_symbol_table(exprBlur->SymbolTable);
	if (!exprBlur->compileFormula())
		return false;

	// Load Blur shader
	shader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[1]);
	if (!shader)
		return false;

	// Configure Blur shader
	shader->use();
	shader->setValue("image", 0);	// The image is in the texture unit 0

	shaderVars = new ShaderVars(this, shader);
	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		shaderVars->ReadString(uniform[i].c_str());
	}
	// Set shader variables values
	shaderVars->setValues();
	
	return true;
}

void sEfxBlur::init() {
	
}

void sEfxBlur::exec() {
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (clearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (clearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Evaluate the expression
	exprBlur->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// First step: Blur the image from the "fbo attachment 0", and store it in our efxBloom fbo manager (efxBloomFbo)
		bool horizontal = true;
		bool first_iteration = true;
		shader->use();

		// Prevent negative Blurs
		if (blurAmount < 0)
			blurAmount = 0;
		unsigned int iBlurAmount = static_cast<unsigned int>(blurAmount);
		for (unsigned int i = 0; i < iBlurAmount; i++)
		{
			shader->setValue("horizontal", horizontal);
			
			// We always draw the First pass in the efxBloom FBO
			m_demo.efxBloomFbo.bind(horizontal, false, false); // TODO: Fix: use an FBO for Blur, not the Bloom FBO
			
			// If it's the first iteration, we pick the fbo
			// if not, we pick the fbo of our efxBloom
			if (first_iteration)
				m_demo.fboManager.bind_tex(FboNum);
			else
				m_demo.efxBloomFbo.bind_tex(!horizontal);
			
			// Render scene
			m_demo.res->Draw_QuadFS();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}

		m_demo.efxBloomFbo.unbind(false, false); // Unbind drawing into an Fbo
		
		// Adjust back the current fbo
		m_demo.fboManager.bindCurrent();
		// Second step: Draw the Blurred image
		m_demo.res->Draw_QuadEfxFBOFS(!horizontal);
	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxBlur::end() {
	
}

std::string sEfxBlur::debug() {
	std::string msg;
	msg = "[ efxBlur id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " fbo: " + std::to_string(FboNum) + " Blur Amount: " + std::to_string(blurAmount) + "\n";
	return msg;
}
