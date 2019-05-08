// fbo.h
// Spontz Demogroup

#ifndef FBO_H
#define FBO_H
#include <string>
#include <glad/glad.h>
using namespace std;

// flags for texture properties
#define NO_MIPMAP 1
#define CLAMP_TO_EDGE 2
#define CLAMP 4
#define MODULATE 8
#define TEXTURE_1D 16

class Fbo {
public:
	
	int width, height;
	int iformat, format, ttype, components;
	bool use_linear;
	GLuint fboID;				// Our handle to the FBO
	GLuint depthBufferID;		// Our handle to the depth render buffer
	GLuint textureID;			// Our handle to the Open GL texture ID

	Fbo();
	virtual ~Fbo();

	bool upload(int index, int width, int height, int iformat, int format, int type);
	void bind() const;
	void bind_tex() const;
	void active(int index = 0) const;
				
private:


};

#endif