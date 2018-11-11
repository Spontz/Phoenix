#include "main.h"

typedef enum
{
	background_drawing_mode__fit_to_viewport = 0,	// ajusta la imagen para que ocupe todo el viewport (sin distorsion de aspect)
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
	case background_drawing_mode__fit_to_content:
		local->mode = background_drawing_mode__fit_to_content;
		break;
	case background_drawing_mode__fit_to_viewport:
		local->mode = background_drawing_mode__fit_to_viewport;
		break;
	default:
		LOG->Error("Background [%s]: Invalid value for param[0]", this->identifier.c_str());
	}

	// texture loading
	string s_dir = DEMO->demoDir;
	local->texture = DEMO->textureManager.addTexture(s_dir + this->strings[0]);
	if (local->texture == -1)
		return;
}

void sBackground::init() {
}

void sBackground::exec() {
//	int i;

	local = (background_section *)this->vars;

	glDisable(GL_DEPTH_TEST);
	{
		if (this->hasBlend)
		{
			glBlendFunc(this->sfactor, this->dfactor);
			glEnable(GL_BLEND);
		}

		if (this->hasAlpha)
		{
			float alpha = this->alpha1 + this->runTime * (this->alpha2 - this->alpha1) / this->duration;
			
			//glEnable(GL_ALPHA_TEST);
			//glAlphaFunc(this->alphaFunc, alpha);
		}
		// compute rectangle coordinates
		{
			// hack : las texturas se cargan con resoluciones multiplo de dos
//			float TextureAspectRatio = (float)tex_array[local->texture]->width / (float)tex_array[local->texture]->height;

			switch (local->mode)
			{
			case background_drawing_mode__fit_to_viewport:
//				camera_2d_fit_to_viewport(TextureAspectRatio, &local->quad.x0, &local->quad.x1, &local->quad.y0, &local->quad.y1);
				break;
			case background_drawing_mode__fit_to_content:
//				camera_2d_fit_to_content(TextureAspectRatio, &local->quad.x0, &local->quad.x1, &local->quad.y0, &local->quad.y1);
				break;
			default:
				LOG->Error("Invalid background section drawing mode");
				break;
			}
		}

//		camera_set2d();
		// Rotate our "world"
		/*
		DEMO->cam_position = glm::vec3(0.0f, 1.0f, 1.2f);
		cam_look_at = glm::vec3(0.0f, 0.5f, 0.0f);
		cam_up = glm::vec3(0.0f, 1.0f, 0.0f);

		view_matrix = glm::lookAt(cam_position, cam_look_at, cam_up);
		projection_matrix = glm::perspectiveFov(glm::radians(90.0f), float(GLDRV->width), float(GLDRV->height), 0.1f, 10.0f);

		if (GLDRV->width >= 1 && GLDRV->height >= 1) {
			GLDRV->projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(GLDRV->width), float(GLDRV->height), 0.1f, 10.0f);
			if (shader != -1) {
				DEMO->shaderManager.shader[shader]->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);
			}
		}
		*/
		Shader *my_shad = DEMO->shaderManager.shader[RES->shdr_basic];
		// Place the quad in the center
		glm::mat4 world_matrix = glm::mat4(1.0f);
		my_shad->setUniformMatrix4fv("world", world_matrix);
		my_shad->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(world_matrix))));

		// Put the camera in 2D mode
		// Init matrices
		glm::vec3 cam_position = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 cam_look_at = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 projection_matrix = glm::perspectiveFov(glm::radians(90.0f), float(GLDRV->width), float(GLDRV->height), 0.1f, 10.0f);
		glm::mat4 view_matrix = glm::lookAt(cam_position, cam_look_at, cam_up);
		my_shad->setUniformMatrix4fv("viewProj", projection_matrix * view_matrix);
		

		RES->Draw_Obj_Quad();

//		camera_restore();

//		if (this->hasAlpha)
//			glDisable(GL_ALPHA_TEST);

		if (this->hasBlend)
			glDisable(GL_BLEND);
	}
	glEnable(GL_DEPTH_TEST);
}



void sBackground::end() {

}
