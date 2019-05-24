#include "main.h"

sLoading::sLoading() {
	type = SectionType::Loading;
}

bool sLoading::load() {
	return true;
}

void sLoading::init() {
	
}

void sLoading::exec() {
	GLDRV->initRender(true);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_DEPTH_TEST);
	RES->Draw_QuadFS(RES->tex_LoadingFront);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	

	GLDRV->swap_buffers();
}

void sLoading::end() {
	
}
