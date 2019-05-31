// fbomanager.cpp
// Spontz Demogroup

#include "main.h"
#include "fbomanager.h"


// Init vars
FboManager::FboManager() {
	fbo.clear(); 
	mem = 0;
}

void FboManager::active(int index) const
{
	glActiveTexture(GL_TEXTURE0 + index);
}

void FboManager::bind(int fbo_num)
{
	if (fbo_num < fbo.size()) {
		Fbo *my_fbo = fbo[fbo_num];
		// Adjust the viewport to the fbo size
		GLDRV->setViewport(0, 0, my_fbo->width, my_fbo->height);
		my_fbo->bind();
	}
}

void FboManager::bind_tex(int fbo_num)
{
	if (fbo_num < fbo.size()) {
		Fbo *my_fbo = fbo[fbo_num];
		my_fbo->bind_tex();
	}
}

void FboManager::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Restore the driver viewport
	GLDRV->setViewport(0, 0, GLDRV->width, GLDRV->height);
}

// Adds a Fbo into the queue, returns the ID of the texture added
int FboManager::addFbo(string engine_format, int width, int height, int iformat, int format, int type, int components) {
	int fbo_id = -1;
	Fbo *new_fbo = new Fbo();
	if (new_fbo->upload(engine_format, (int)fbo.size(), width, height, iformat, format, type)) {
		fbo.push_back(new_fbo);
		mem += (float)(new_fbo->width * new_fbo->height * components) / 1048576.0f;		// increase the texture mem
		fbo_id = (int)fbo.size() - 1;
		LOG->Info(LOG_MED, "Fbo Color %d loaded OK. Overall fbo Memory: %.3fMb", fbo_id, mem);
	}
	else {
		LOG->Error("Could not load fbo with the current format: Width: %d, Height: %d, iformat: %d, format: %d, type: %d", width, height, iformat, format, type);
		return -1;
	}
	return fbo_id;
}

int FboManager::getOpenGLTextureID(int index)
{
	if (index < fbo.size())
		return fbo[index]->textureBufferID;
	else
		return -1;
}
