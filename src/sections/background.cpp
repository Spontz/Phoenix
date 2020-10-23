#include "main.h"

enum class e_background_drawing_mode : unsigned int {
	fit_to_viewport = 0, 	// adjust the image to fit all the viewport, but it may break the image aspect ratio
	fit_to_content = 1		// adjust the image in order to be seen completelly, but keeping the image aspect ratio
};

struct sBackground : public Section {
public:
	sBackground();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	e_background_drawing_mode m_eMode		= e_background_drawing_mode::fit_to_viewport;

	float		m_fTexAspectRatio = 1.0f;
	Texture*	m_pTexture					= nullptr;
	Shader*		m_pShader					= nullptr;
};

// ******************************************************************

Section* instance_background()
{
	return new sBackground();
}

sBackground::sBackground()
{
	type = SectionType::Background;
}

bool sBackground::load() {
	// script validation
	if ((strings.size() != 1) || (param.size() != 1)) {
		LOG->Error("Background [%s]: 1 string and 1 param needed", identifier.c_str());
		return false;
	}

	m_eMode = static_cast<e_background_drawing_mode>(param[0]);

	switch (m_eMode) {
	case e_background_drawing_mode::fit_to_viewport:
	case e_background_drawing_mode::fit_to_content:
		break;

	default:
		LOG->Error("Background [%s]: Invalid value for param[0]", identifier.c_str());
	}

	// Load the shader for drawing the quad
	m_pShader = m_demo.shaderManager.addShader(m_demo.dataFolder + "/resources/shaders/sections/background.glsl");
	if (!m_pShader)
		return false;
	// Background texture load
	m_pTexture = m_demo.textureManager.addTexture(m_demo.dataFolder + strings[0]);
	if (!m_pTexture)
		return false;
	m_fTexAspectRatio = static_cast<float>(m_pTexture->width) / static_cast<float>(m_pTexture->height);

	return true;
}

void sBackground::init() {
}

void sBackground::exec() {
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// View aspect ratio, stored for Keeping image proportions
		const float fViewportAspect = GLDRV->GetCurrentViewport().GetAspectRatio();

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float fXScale = 1;
		float fYScale = 1;
		if (m_eMode == e_background_drawing_mode::fit_to_content) {
			if (m_fTexAspectRatio > fViewportAspect)
				fYScale = fViewportAspect / m_fTexAspectRatio;
			else
				fXScale = m_fTexAspectRatio / fViewportAspect;
		}
		const glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(fXScale, fYScale, 0.0f));

		m_pShader->use();
		m_pShader->setValue("model", modelMatrix);
		m_pShader->setValue("screenTexture", 0);
		m_pTexture->bind(0);
		m_demo.res->Draw_QuadFS();
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sBackground::end() {
}

std::string sBackground::debug() {
	std::stringstream ss;
	ss << "+ Background id: " << identifier << " layer: " << layer << std::endl;
	ss << "  file: " << m_pTexture->filename << std::endl;
	return ss.str();
}
