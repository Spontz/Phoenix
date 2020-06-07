#include "main.h"

enum class e_background_drawing_mode : unsigned int {
	// adjust the image to fit all the viewport, but it may break the image aspect ratio
	fit_to_viewport = 0,
	// adjust the image in order to be seen completelly, but keeping the image aspect ratio
	fit_to_content = 1
};

struct sBackground : public Section {
	public:
	sBackground();
	bool load();
	void init();
	void exec();
	void end();
	string debug();

	private:
	int texture_id_;
	int shader_id_;
	e_background_drawing_mode mode_;
};

Section* instance_background() {
	return new sBackground();
}

sBackground::sBackground(){
	type = SectionType::Background;
}

bool sBackground::load() {
	// script validation
	if ((strings.size() != 1) || (param.size() != 1)) {
		LOG->Error("Background [%s]: 1 string and 1 param needed", identifier.c_str());
		return false;
	}

	mode_ = static_cast<e_background_drawing_mode>(param[0]);

	// vars = this;

	switch (mode_) {
	case e_background_drawing_mode::fit_to_viewport:
	case e_background_drawing_mode::fit_to_content:
		break;

	default:
		LOG->Error("Background [%s]: Invalid value for param[0]", identifier.c_str());
	}

	// Load the shader for drawing the quad
	shader_id_ = DEMO->shaderManager.addShader(
		DEMO->dataFolder + "/resources/shaders/sections/background_texquad.vert",
		DEMO->dataFolder + "/resources/shaders/sections/background_texquad.frag"
	);
	// Background texture load
	texture_id_ = DEMO->textureManager.addTexture(DEMO->dataFolder + strings[0]);
	if (texture_id_ == -1)
		return false;
	return true;
}

void sBackground::init() {
}

void sBackground::exec() {
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Load the background texture
		const Texture* const p_texture = DEMO->textureManager.texture[texture_id_];
		// Texture and View aspect ratio, stored for Keeping image proportions
		const float tex_aspect = static_cast<float>(p_texture->width) / static_cast<float>(p_texture->height);
		const float viewport_aspect = GLDRV->GetCurrentViewport().GetAspectRatio();

		// Put orthogonal mode

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float new_tex_width_scaled = 1;
		float new_tex_height_scaled = 1;
		if (mode_ == e_background_drawing_mode::fit_to_content) {
			if (tex_aspect > viewport_aspect)
				new_tex_height_scaled = viewport_aspect / tex_aspect;
			else
				new_tex_width_scaled = tex_aspect / viewport_aspect;
		}
		const glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(new_tex_width_scaled, new_tex_height_scaled, 0.0f));

		RES->Draw_Obj_QuadTex(texture_id_, &model);
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
}

void sBackground::end() {
}

string sBackground::debug() {
	Texture* my_tex;
	my_tex = DEMO->textureManager.texture[texture_id_];

	string msg;
	msg = "[ background id: " + identifier + " layer:" + std::to_string(layer) + " ]\n";
	msg += " filename: " + my_tex->filename + "\n";
	return msg;
}
