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
	int i;

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

		/*
		// compute rectangle coordinates
		{
			// hack : las texturas se cargan con resoluciones multiplo de dos
			float TextureAspectRatio = (float)tex_array[local->texture]->width / (float)tex_array[local->texture]->height;

			switch (local->mode)
			{
			case background_drawing_mode__fit_to_viewport:
				camera_2d_fit_to_viewport(TextureAspectRatio, &local->quad.x0, &local->quad.x1, &local->quad.y0, &local->quad.y1);
				break;
			case background_drawing_mode__fit_to_content:
				camera_2d_fit_to_content(TextureAspectRatio, &local->quad.x0, &local->quad.x1, &local->quad.y0, &local->quad.y1);
				break;
			default:
				section_error("Invalid background section drawing mode");
				break;
			}
		}

		// store vertex values
		local->quad.pc[0][0] = local->quad.x0;
		local->quad.pc[1][0] = local->quad.x1;
		local->quad.pc[2][0] = local->quad.x1;
		local->quad.pc[3][0] = local->quad.x0;

		local->quad.pc[0][1] = local->quad.y0;
		local->quad.pc[1][1] = local->quad.y0;
		local->quad.pc[2][1] = local->quad.y1;
		local->quad.pc[3][1] = local->quad.y1;

		local->quad.tc[0][0] = 0; local->quad.tc[0][1] = 1;
		local->quad.tc[1][0] = 1; local->quad.tc[1][1] = 1;
		local->quad.tc[2][0] = 1; local->quad.tc[2][1] = 0;
		local->quad.tc[3][0] = 0; local->quad.tc[3][1] = 0;

		camera_set2d();

		tex_bind(local->texture);
		glBegin(GL_QUADS);
		for (i = 0; i < 4; i++)
		{
			glTexCoord2fv(local->quad.tc[i]);
			glVertex2fv(local->quad.pc[i]);
		}
		glEnd();

		camera_restore();

		if (this->hasAlpha)
			glDisable(GL_ALPHA_TEST);

		if (this->hasBlend)
			glDisable(GL_BLEND);
			*/
	}
	glEnable(GL_DEPTH_TEST);
}



void sBackground::end() {

}
