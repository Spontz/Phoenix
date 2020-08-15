#include "main.h"

struct sLoading : public Section {
public:
	sLoading();
	bool		load();
	void		init();
	void		exec();
	void		end();
	std::string debug();

private:
	demokernel& demo = demokernel::GetInstance();
	int tex_front, tex_back, tex_bar;
	
	float	tx, ty;// Bar Translation
	float	sy;// Bar Scale

	bool byDefault;
};

// ******************************************************************

Section* instance_loading() {
	return new sLoading();
}

sLoading::sLoading() {
	type = SectionType::Loading;
}

bool sLoading::load() {
	// script validation
	if ((this->param.size() != 3) || (this->strings.size() != 3)) {
		LOG->Error("Loading [%s]: 3 strings and 3 params needed. Using default values.", this->identifier.c_str());
		byDefault = true;
	}
	else {
		byDefault = false;
	}

	if (!byDefault) {
		tex_back = demo.textureManager.addTexture(demo.dataFolder + strings[0]);
		tex_front = demo.textureManager.addTexture(demo.dataFolder + strings[1]);
		tex_bar = demo.textureManager.addTexture(demo.dataFolder + strings[2]);
		tx = param[0];
		ty = param[1];
		sy = param[2];
	}
	else {
		// Deault values
		tex_back = demo.textureManager.addTexture(demo.dataFolder + "/resources/loading/loadingback.jpg");
		tex_front = demo.textureManager.addTexture(demo.dataFolder + "/resources/loading/loadingfront.jpg");
		tex_bar = demo.textureManager.addTexture(demo.dataFolder + "/resources/loading/loadingbar.jpg");
		tx = 0.0f;
		ty = -0.4f;
		sy = 0.1f;
	}

	if (tex_bar == -1 || tex_back == -1 || tex_front == -1) {
		LOG->Error("Loading [%s]: Could not load some of the loading textures", identifier.c_str());
	}

	return true;
}

void sLoading::init() {
	
}

void sLoading::exec() {
	// Prevent a crash if any texture is not found
	if (tex_bar == -1 || tex_back == -1 || tex_front == -1) {
		return;
	}

	float zero2one = 0;
	if (demo.sectionManager.loadSection.size()>0)
		zero2one = (float)demo.loadedSections / (float)(demo.sectionManager.loadSection.size());
	
	GLDRV->initRender(true);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
		// Background
		RES->Draw_QuadFS(tex_back, 1 - zero2one);
		// Foreground
		RES->Draw_QuadFS(tex_front, zero2one);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	
	// Draw the Loading bar
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(tx, ty, 0));  // Move the bar
	model = glm::scale(model, glm::vec3(zero2one, sy, 0));		// Scale the bar
	RES->Draw_Obj_QuadTex(tex_bar, &model);

	GLDRV->swapBuffers();
}

void sLoading::end() {
	
}

std::string sLoading::debug() {
	return std::string();
}
