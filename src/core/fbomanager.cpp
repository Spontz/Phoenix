// fbomanager.cpp
// Spontz Demogroup

#include "main.h"
#include "fbomanager.h"


// Init vars
FboManager::FboManager() {
	fbo.clear();
	mem = 0;
	currentFbo = -1;
	clearColor = false;
	clearDepth = false;
}

void FboManager::active(int index) const
{
	glActiveTexture(GL_TEXTURE0 + index);
}

void FboManager::bind(int fbo_num, bool clearColor, bool clearDepth)
{
	if (fbo_num < fbo.size()) {
		this->currentFbo = fbo_num;
		this->clearColor = clearColor;
		this->clearDepth = clearDepth;
		Fbo* my_fbo = fbo[fbo_num];
		// Adjust the viewport to the fbo size
		GLDRV->SetCurrentViewport({0,0,static_cast<unsigned int>(my_fbo->width),static_cast<unsigned int>(my_fbo->height)});
		my_fbo->bind();
		if (clearColor)	glClear(GL_COLOR_BUFFER_BIT);
		if (clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
	}
}

// Bind the current fbo: useful for some efects that need to change the framebuffer (like Boom or blur)
void FboManager::bindCurrent()
{
	if (this->currentFbo>=0) {
		Fbo* my_fbo = fbo[this->currentFbo];
		// Adjust the viewport to the fbo size
		GLDRV->SetCurrentViewport({ 0,0,static_cast<unsigned int>(my_fbo->width),static_cast<unsigned int>(my_fbo->height) });
		my_fbo->bind();
		if (clearColor)	glClear(GL_COLOR_BUFFER_BIT);
		if (clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
	}
	else{
		DEMO->fboManager.unbind(clearColor, clearDepth);
	}
}

void FboManager::bind_tex(int fbo_num, GLuint attachment)
{
	if (fbo_num < fbo.size()) {
		Fbo* my_fbo = fbo[fbo_num];
		my_fbo->bind_tex(attachment);
	}
}

void FboManager::unbind(bool clearColor, bool clearDepth)
{
	currentFbo = -1;
	this->clearColor = clearColor;
	this->clearDepth = clearDepth;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Restore the driver viewport
	GLDRV->SetCurrentViewport(GLDRV->GetFramebufferViewport());
	if (clearColor)	glClear(GL_COLOR_BUFFER_BIT);
	if (clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
}

// Adds a Fbo into the queue, returns the ID of the texture added
int FboManager::addFbo(std::string engine_format, int width, int height, int iformat, int format, int type, int components, unsigned int numColorAttachments) {
	int fbo_id = -1;
	Fbo* new_fbo = new Fbo();
	if (new_fbo->upload(engine_format, (int)fbo.size(), width, height, iformat, format, type, numColorAttachments)) {
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

void FboManager::clearFbos()
{
	for (int i = 0; i < this->fbo.size(); i++) {
		delete this->fbo[i];
	}
	this->fbo.clear();
	this->mem = 0;
}

int FboManager::getOpenGLTextureID(unsigned int index, unsigned int attachment)
{
	if (index < fbo.size()) {
		if (attachment < fbo[index]->numAttachments) {
			return fbo[index]->colorBufferID[attachment];
		}
		else
			return -1;
	}
	else
		return -1;
}
