#include "main.h"

typedef enum
{
	background_drawing_mode__fit_to_viewport = 0,	// ajusta la imagen para que ocupe todo el viewport (con distorsion de aspect)
	background_drawing_mode__fit_to_content = 1		// ajusta la imagen para se vea completa (sin distorsion de aspect)
} enum_background_drawing_mode;

typedef struct {
	int								texture;
	enum_background_drawing_mode	mode;
} background_section;

static background_section *local;

sBackground::sBackground() {
	type = SectionType::Background;
}

void sBackground::load() {
	// script validation
	if (this->stringNum != 1) {
		LOG->Error("Background [%s]: 1 string needed", this->identifier.c_str());
		return;
	}

	if (this->paramNum != 1) {
		LOG->Error("Background [%s]: 1 param needed", this->identifier.c_str());
		return;
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

	// Background texture load
	string s_dir = DEMO->demoDir;
	local->texture = DEMO->textureManager.addTexture(s_dir + this->strings[0], true);
	if (local->texture == -1)
		return;
}

void sBackground::init() {
}

void sBackground::exec() {
	local = (background_section *)this->vars;
	
	glDisable(GL_DEPTH_TEST);
	{
		if (this->hasBlend)
		{
			glBlendFunc(this->sfactor, this->dfactor);
			glEnable(GL_BLEND);
		}

		// Load the Basic Shader
		Shader *my_shad = DEMO->shaderManager.shader[RES->shdr_basic];
		// Load the background texture
		Texture *my_tex = DEMO->textureManager.texture[local->texture];

		my_shad->use();
		my_shad->setInt("texture_diffuse1", 0);

		// Texture and View aspect ratio, stored for Keeping image proportions
		float tex_aspect = (float)my_tex->width / (float)my_tex->height;
		float view_aspect = (float)GLDRV->width / (float)GLDRV->height;

		
		
		// Put orthogonal mode
		glm::mat4 projection = DEMO->camera->getOrthoMatrix_Projection();
		glm::mat4 view = DEMO->camera->getOrthoMatrix_View();
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
		

		// Send matrices to shader
		my_shad->setMat4("model", model);
		my_shad->setMat4("view", view);
		my_shad->setMat4("projection", projection);

		my_tex->active();
		my_tex->bind();

		RES->Draw_Obj_Quad();
		

		if (this->hasBlend)
			glDisable(GL_BLEND);
	}
	glEnable(GL_DEPTH_TEST);
}



void sBackground::end() {
	LOG->Info(LOG_HIGH, "Background has finished!");
}
