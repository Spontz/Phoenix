#include "main.h"

typedef enum
{
	background_drawing_mode__fit_to_viewport = 0,	// adjust the image to fit all the viewport, but it may break the image aspect ratio
	background_drawing_mode__fit_to_content = 1		// adjust the image in order to be seen completelly, but keeping the image aspect ratio
} enum_background_drawing_mode;

typedef struct {
	int								texture;
	int								shader;
	enum_background_drawing_mode	mode;
} background_section;

static background_section *local;

sBackground::sBackground() {
	type = SectionType::Background;
}

bool sBackground::load() {
	// script validation
	if ((this->stringNum != 1) || (this->paramNum != 1)) {
		LOG->Error("Background [%s]: 1 string and 1 param needed", this->identifier.c_str());
		return false;
	}

	local = (background_section*)malloc(sizeof(background_section));
	this->vars = (void *)local;

	switch ((unsigned int)this->param[0]) {
	case background_drawing_mode__fit_to_viewport:
		local->mode = background_drawing_mode__fit_to_viewport;
		break;
	case background_drawing_mode__fit_to_content:
		local->mode = background_drawing_mode__fit_to_content;
		break;
	default:
		LOG->Error("Background [%s]: Invalid value for param[0]", this->identifier.c_str());
	}

	string s_dir = DEMO->demoDir;

	// Load the shader for drawing the quad
	local->shader = DEMO->shaderManager.addShader(s_dir + "/resources/shaders/sections/background_texquad.vert", s_dir + "/resources/shaders/sections/background_texquad.frag");
	// Background texture load
	local->texture = DEMO->textureManager.addTexture(s_dir + this->strings[0], true);
	if (local->texture == -1)
		return false;
	return true;
}

void sBackground::init() {
}

void sBackground::exec() {
	local = (background_section *)this->vars;
	
	if (this->hasBlend) {
		glEnable(GL_BLEND);
		glBlendFunc(this->sfactor, this->dfactor);
	}

	glDisable(GL_DEPTH_TEST);
	{
		// Load the background texture
		Texture *my_tex = DEMO->textureManager.texture[local->texture];
		// Texture and View aspect ratio, stored for Keeping image proportions
		float tex_aspect = (float)my_tex->width / (float)my_tex->height;
		float view_aspect = (float)GLDRV->width / (float)GLDRV->height;

		// Put orthogonal mode
		glm::mat4 model = glm::mat4(1.0f);

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float new_tex_width_scaled = 1;
		float new_tex_height_scaled = 1;
		if (local->mode == background_drawing_mode__fit_to_content) {
			if (tex_aspect > view_aspect) {
				new_tex_height_scaled = view_aspect / tex_aspect;
			}
			else {
				new_tex_width_scaled = tex_aspect / view_aspect;
			}
		}
		model = glm::scale(model, glm::vec3(new_tex_width_scaled, new_tex_height_scaled, 0.0f));
		
		RES->Draw_Obj_QuadTex(local->texture, &model);
	}
	glEnable(GL_DEPTH_TEST);

	if (this->hasBlend)
		glDisable(GL_BLEND);
}

void sBackground::end() {
	LOG->Info(LOG_HIGH, "Background has finished!");
}
