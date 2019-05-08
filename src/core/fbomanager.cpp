// fbomanager.cpp
// Spontz Demogroup

#include "main.h"
#include "fbomanager.h"


// Init vars
FboManager::FboManager() {
	fbo.clear(); 
	mem = 0;
}

// Adds a Fbo into the queue, returns the ID of the texture added
int FboManager::addFbo(int width, int height, int iformat, int format, int type, int components) {
	int fbo_id = -1;

	Fbo *new_fbo = new Fbo();
	if (new_fbo->upload((int)fbo.size(), width, height, iformat, format, type)) {
		fbo.push_back(new_fbo);
		mem += (float)(new_fbo->width * new_fbo->height * components)/ 1048576.0f;		// increase the texture mem
		fbo_id = (int)fbo.size() - 1;
		LOG->Info(LOG_MED, "Fbo %d loaded OK. Overall fbo Memory: %.3fMb", fbo_id, mem);
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
		return fbo[index]->textureID;
	else
		return -1;
}
