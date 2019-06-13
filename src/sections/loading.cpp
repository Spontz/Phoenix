#include "main.h"

// ******************************************************************

typedef struct {
	int tex_front, tex_back, tex_bar;
	
	float	tx, ty;// Bar Translation
	float	sy;// Bar Scale

	int byDefault;
} loading_section;

static loading_section *local;
static char byDefault;

// ******************************************************************

sLoading::sLoading() {
	type = SectionType::Loading;
}

bool sLoading::load() {
	// script validation
	if ((this->param.size() != 3) || (this->strings.size() != 3)) {
		LOG->Error("Loading [%s]: 3 strings and 3 params needed. Using default values.", this->identifier.c_str());
		byDefault = 1;
	}
	else {
		byDefault = 0;
	}

	local = (loading_section*)malloc(sizeof(loading_section));
	this->vars = (void *)local;

	if (!byDefault) {
		local->tex_back = DEMO->textureManager.addTexture(this->strings[0]);
		local->tex_front = DEMO->textureManager.addTexture(this->strings[1]); 
		local->tex_bar = DEMO->textureManager.addTexture(this->strings[2]);
		local->tx = this->param[0];
		local->ty = this->param[1];
		local->sy = this->param[2];
	}
	else {
		// Deault values
		local->tex_back = DEMO->textureManager.addTexture("/resources/loading/loadingback.jpg"); 
		local->tex_front = DEMO->textureManager.addTexture("/resources/loading/loadingfront.jpg");
		local->tex_bar = DEMO->textureManager.addTexture("/resources/loading/loadingbar.jpg");
		local->tx = 0.0f;
		local->ty = -0.4f;
		local->sy = 0.1f;
	}


	return true;
}

void sLoading::init() {
	
}

void sLoading::exec() {
	local = (loading_section *)this->vars;

	float zero2one = 0;
	if (DEMO->sectionManager.loadSection.size()>0)
		zero2one = (float)DEMO->loadedSections / (float)(DEMO->sectionManager.loadSection.size());
	
	GLDRV->initRender(true);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
		// Background
		RES->Draw_QuadFS(local->tex_back, 1 - zero2one);
		// Foreground
		RES->Draw_QuadFS(local->tex_front, zero2one);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	
	// Draw the Loading bar
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(local->tx, local->ty, 0));  // Move the bar
	model = glm::scale(model, glm::vec3(zero2one, local->sy, 0));		// Scale the bar
	RES->Draw_Obj_QuadTex(local->tex_bar, &model);

	GLDRV->swap_buffers();
}

void sLoading::end() {
	
}
