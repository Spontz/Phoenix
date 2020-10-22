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
	return true;
}

void sBackground::init() {
}

void sBackground::exec() {
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Texture and View aspect ratio, stored for Keeping image proportions
		const float tex_aspect = static_cast<float>(m_pTexture->width) / static_cast<float>(m_pTexture->height);
		const float viewport_aspect = GLDRV->GetCurrentViewport().GetAspectRatio();

		// Put orthogonal mode

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float new_tex_width_scaled = 1;
		float new_tex_height_scaled = 1;
		if (m_eMode == e_background_drawing_mode::fit_to_content) {
			if (tex_aspect > viewport_aspect)
				new_tex_height_scaled = viewport_aspect / tex_aspect;
			else
				new_tex_width_scaled = tex_aspect / viewport_aspect;
		}
		const glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(new_tex_width_scaled, new_tex_height_scaled, 0.0f));

		m_pShader->use();
		m_pShader->setValue("model", model);
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
