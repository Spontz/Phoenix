#include "main.h"
#include "core/drivers/mathdriver.h"
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
	bool			m_bClearScreen		= true;		// Clear Screen buffer
	bool			m_bClearDepth		= false;	// Clear Depth buffer
	unsigned int	m_uiFboNum			= 0;		// Fbo to use (must have 2 color attachments!)
	float			m_fSourceInfluence	= 0.5f;		// Source influence (0 to 1)
	float			m_fAccumInfluence	= 0.5f;		// Accumulation influence (0 to 1)
	bool			m_bAccumBuffer		= false;	// Accum buffer to use (0 or 1)
	Shader			*m_pShader			= nullptr;	// Accumulation Shader to apply
	mathDriver		*m_pExprAccum		= nullptr;	// Equations for the Accum effect
	ShaderVars		*m_pVars			= nullptr;	// Shader variables
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
	m_bClearScreen = static_cast<bool>(param[0]);
	m_bClearDepth = static_cast<bool>(param[1]);
	m_uiFboNum = static_cast<unsigned int>(param[2]);
	m_bAccumBuffer = false;
	
	// Check if the fbo can be used for the effect
	if (m_uiFboNum < 0 || m_uiFboNum >= m_demo.m_fboManager.fbo.size()) {
		LOG->Error("EfxBlur [%s]: The fbo specified [%d] is not supported, should be between 0 and %d", identifier.c_str(), m_uiFboNum, m_demo.m_fboManager.fbo.size()-1);
		return false;
	}
	
	// Load the Blur amount formula
	m_pExprAccum = new mathDriver(this);
	// Load positions, process constants and compile expression
	for (int i = 1; i < strings.size(); i++)
		m_pExprAccum->expression += strings[i];
	m_pExprAccum->SymbolTable.add_variable("SourceInfluence", m_fSourceInfluence);
	m_pExprAccum->SymbolTable.add_variable("AccumInfluence", m_fAccumInfluence);
	m_pExprAccum->Expression.register_symbol_table(m_pExprAccum->SymbolTable);
	if (!m_pExprAccum->compileFormula())
		return false;

	// Load Blur shader
	m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + strings[0]);
	if (!m_pShader)
		return false;

	// Configure shader
	m_pShader->use();
	m_pShader->setValue("sourceImage", 0);	// The source image will be in the texture unit 0
	m_pShader->setValue("accumImage", 1);	// The accumulated image will be in the texture unit 1

	m_pVars = new ShaderVars(this, m_pShader);
	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		m_pVars->ReadString(uniform[i].c_str());
	}
	// Set shader variables values
	m_pVars->setValues();
	
	return true;
}

void sEfxAccum::init() {
	
}

static float lastTime = 0;
static bool firstIteration = true;

void sEfxAccum::exec() {
	// Clear the screen and depth buffers depending of the parameters passed by the user
	if (m_bClearScreen) glClear(GL_COLOR_BUFFER_BIT);
	if (m_bClearDepth) glClear(GL_DEPTH_BUFFER_BIT);

	// Calculate deltaTime
	float deltaTime = runTime - lastTime;
	lastTime = runTime;

	// Evaluate the expression
	m_pExprAccum->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		
		{
			// We want to capture the frame in the "Accum Fbo", so first we use the previous fbo for storing the entire image
			m_demo.m_efxAccumFbo.bind(m_bAccumBuffer, false, false);
		
			float fps = 1.0f / 60.0f;
			m_pShader->use();
			m_pShader->setValue("sourceInfluence", m_fSourceInfluence * (deltaTime/fps) );
			m_pShader->setValue("accumInfluence", 1-(m_fAccumInfluence * (deltaTime/fps)) );
			m_pVars->setValues();

			// Set the screen fbo in texture unit 0
			m_demo.m_fboManager.bind_tex(m_uiFboNum, 0);
			
			// Set the accumulation fbo in texture unit 1
			if (firstIteration)
				firstIteration = false;
			m_bAccumBuffer = !m_bAccumBuffer; 
			m_demo.m_efxAccumFbo.bind_tex(m_bAccumBuffer, 1);

			// Render a quad using the Accum shader (combining the 2 Images)
			m_demo.m_pRes->Draw_QuadFS();

			m_demo.m_efxAccumFbo.unbind(false, false); // Unbind drawing into the "Accum Fbo"

			// Adjust back the current fbo
			m_demo.m_fboManager.bindCurrent();
		}
		

		// Second step: Draw the accum buffer
		m_demo.m_pRes->shdr_QuadTex->use();
		m_demo.m_pRes->shdr_QuadTex->setValue("screenTexture", 0);
		if (firstIteration)
			m_demo.m_fboManager.bind_tex(m_uiFboNum, 0);
		else
			m_demo.m_efxAccumFbo.bind_tex(!m_bAccumBuffer, 0);
		m_demo.m_pRes->Draw_QuadFS();

	}		
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sEfxAccum::end() {
	
}

std::string sEfxAccum::debug() {
	std::stringstream ss;
	ss << "+ EfxAccum id: " << identifier << " layer: " << layer << std::endl;
	ss << "  fbo: " << m_uiFboNum << " Source Influence: " << m_fSourceInfluence << " Accum Influence: " << m_fAccumInfluence  << std::endl;
	return ss.str();
}
