#include "main.h"
#include "core/drivers/mathdriver.h"
#include "core/shadervars.h"

struct sDrawImage : public Section {
public:
	sDrawImage();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	float		m_fTexAspectRatio	= 1.0f;

	glm::vec3	m_vTranslation		= { 0, 0, 0 };
	glm::vec3	m_vRotation			= { 0, 0, 0 };
	glm::vec3	m_vScale			= { 1, 1, 1 };

	Texture		*m_pTexture			= nullptr;
	Shader		*m_pShader			= nullptr;
	mathDriver	*m_pExprPosition	= nullptr;	// A equation containing the calculations to position the object
	ShaderVars	*m_pVars			= nullptr;	// For storing any other shader variables
} drawImage_section;

// ******************************************************************

Section* instance_drawImage() {
	return new sDrawImage();
}

sDrawImage::sDrawImage() {
	type = SectionType::DrawImage;
}


bool sDrawImage::load() {
	if (strings.size() < 5) {
		LOG->Error("Draw Image [%s]: 5 strings required (image path, shader and 3 por image positioning)", identifier.c_str());
		return false;
	}

	// Texture load
	m_pTexture = m_demo.textureManager.addTexture(m_demo.dataFolder + strings[0]);

	if (!m_pTexture)
		return false;
	// Load the background texture
	m_fTexAspectRatio = (float)m_pTexture->height / (float)m_pTexture->width;

	// Load the shader to apply
	m_pShader = m_demo.shaderManager.addShader(m_demo.dataFolder + strings[1]);
	if (!m_pShader)
		return false;

	// Load the formmula containing the Image position and scale
	m_pExprPosition = new mathDriver(this);
	// Load positions, process constants and compile expression
	for (int i = 2; i < strings.size(); i++)
		m_pExprPosition->expression += strings[i];
	m_pExprPosition->SymbolTable.add_variable("tx", m_vTranslation.x);
	m_pExprPosition->SymbolTable.add_variable("ty", m_vTranslation.y);
	m_pExprPosition->SymbolTable.add_variable("tz", m_vTranslation.z);
	m_pExprPosition->SymbolTable.add_variable("rx", m_vRotation.x);
	m_pExprPosition->SymbolTable.add_variable("ry", m_vRotation.y);
	m_pExprPosition->SymbolTable.add_variable("rz", m_vRotation.z);
	m_pExprPosition->SymbolTable.add_variable("sx", m_vScale.x);
	m_pExprPosition->SymbolTable.add_variable("sy", m_vScale.y);
	m_pExprPosition->SymbolTable.add_variable("sz", m_vScale.z);
	m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
	if (!m_pExprPosition->compileFormula())
		return false;

	// Create shader variables
	m_pShader->use();
	m_pVars = new ShaderVars(this, m_pShader);

	// Read the shader variables
	for (int i = 0; i < uniform.size(); i++) {
		m_pVars->ReadString(uniform[i].c_str());
	}

	// Set shader variables values
	m_pVars->setValues();

	return true;
}

void sDrawImage::init() {
	
}

void sDrawImage::exec() {
	// Evaluate the expression
	m_pExprPosition->Expression.value();

	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// View / projection / model Matrixes
		glm::mat4 view = m_demo.camera->GetViewMatrix();
		float zoom = m_demo.camera->Zoom;
		glm::mat4 projection = glm::perspective(glm::radians(zoom), GLDRV->GetCurrentViewport().GetAspectRatio(), 0.1f, 10000.0f);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, m_vTranslation);
		model = glm::rotate(model, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(m_vScale.x, m_vScale.y*m_fTexAspectRatio, m_vScale.z));

		// Draw the image
		m_pShader->use();
		m_pShader->setValue("model", model);
		m_pShader->setValue("projection", projection);
		m_pShader->setValue("view", view);
		m_pShader->setValue("screenTexture", 0);

		m_pTexture->bind();

		// Set the values
		m_pVars->setValues();

		m_demo.res->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sDrawImage::end() {
	
}

std::string sDrawImage::debug() {
	std::stringstream ss;
	ss << "+ DrawImage id: " << identifier << " layer: " << layer << std::endl;
	ss << "  file: " << m_pTexture->filename << std::endl;
	return ss.str();
}
