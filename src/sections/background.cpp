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
	local->texture = DEMO->textureManager.addTexture(s_dir + this->strings[0], true);
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
		Shader *my_shad = DEMO->shaderManager.shader[RES->shdr_basic];

		Texture *my_tex = DEMO->textureManager.texture[local->texture];

		my_shad->use();
		my_shad->setInt("texture_diffuse1", 0);

		//glm::mat4 projection = glm::ortho(0.0f,	static_cast<float>(GLDRV->width), static_cast<float>(GLDRV->height), 0.0f, 0.0f, 100.0f);
		//glm::mat4 projection = glm::ortho(-1, 1, 1, -1, 0.0f, 100.0f);
		//glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1.21f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		//glm::mat4 model = glm::mat4(1.0f);
		  //model = glm::translate(model, glm::vec3(static_cast<float>(GLDRV->width)/2.0f, static_cast<float>(GLDRV->height)/2.0f, 0));
		  //model = glm::scale(model, glm::vec3(static_cast<float>(GLDRV->width), static_cast<float>(GLDRV->height), 0));
		  //model = glm::translate(model, glm::vec3(0, 0, 2*sin(DEMO->runTime)));
		
		// Keep image proportions
		float tex_aspect = (float)my_tex->width / (float)my_tex->height;
		float view_aspect = (float)GLDRV->width / (float)GLDRV->height;

		float Left, Right, Bottom, Top;

		if (view_aspect > 1) {
			Left = (1 - view_aspect) / 2;
			Right = (1 - view_aspect) / 2 + view_aspect;
			Bottom = 0;
			Top = 1;
		}
		else {
			Left = 0;
			Right = 1;
			Bottom = (1 - 1 / view_aspect) / 2;
			Top = (1 - 1 / view_aspect) / 2 + 1 / view_aspect;
		}

		//glm::mat4 projection = glm::ortho(-1, 1, 1, -1, -1, 2);
		glm::mat4 projection = glm::ortho(Left, Right, Bottom, Top, -1.0f, 2.0f);

		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(1, scale, 1));

		//model = glm::translate(model, glm::vec3(static_cast<float>(GLDRV->width)/2.0f, static_cast<float>(GLDRV->height)/2.0f, 0));
		//model = glm::scale(model, glm::vec3(static_cast<float>(GLDRV->width), static_cast<float>(GLDRV->height), 0));
		//model = glm::translate(model, glm::vec3(0, 0, 2*sin(DEMO->runTime)));


		// Set matrices to shader
		my_shad->setMat4("model", model);
		my_shad->setMat4("view", view);
		my_shad->setMat4("projection", projection);

		my_tex->active();
		my_tex->bind();

		RES->Draw_Obj_Quad();
		
//		camera_restore();

//		if (this->hasAlpha)
//			glDisable(GL_ALPHA_TEST);

//		if (this->hasBlend)
//			glDisable(GL_BLEND);
	}
	glEnable(GL_DEPTH_TEST);


}



void sBackground::end() {
	LOG->Info(LOG_HIGH, "Background has finished!");
}
