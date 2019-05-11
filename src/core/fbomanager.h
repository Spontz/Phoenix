// fbonmanager.h
// Spontz Demogroup

#ifndef FBOMANAGER_H
#define FBOMANAGER_H
#include <string>
#include <vector>
#include "core/fbo.h"

using namespace std;

class FboManager {
public:
	std::vector<Fbo*> fbo;		// Fbo's list
	float mem;					// Fbo's memory in MegaBytes

	FboManager();
	void active(int index = 0) const;
	void bind(int fbo_num);
	void bind_tex(int fbo_num);
	void unbind();
	int addFbo(int width, int height, int iformat, int format, int type, int components);
	int	getOpenGLTextureID(int index);

private:
};

#endif