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
	float	mem;				// Fbo's memory in MegaBytes
	int		currentFbo;			// Current fbo (-1 if it's the frameBuffer)
	bool	clearColor;			// Clear color bit
	bool	clearDepth;			// Clear depth bit

	FboManager();
	void active(int index = 0) const;
	void bind(int fbo_num, bool clearColor, bool clearDepth);
	void bindCurrent();
	void bind_tex(int fbo_num, GLuint attachment = 0);
	void unbind(bool clearColor, bool clearDepth);
	int addFbo(string engine_format, int width, int height, int iformat, int format, int type, int components, unsigned int numColorAttachments);
	void clearFbos();
	int	getOpenGLTextureID(unsigned int index, unsigned int attachment = 0);

private:
};

#endif