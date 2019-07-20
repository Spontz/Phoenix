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
	unsigned int numAttachments;
	bool use_linear;
	string engineFormat;		// format of the FBO, by a name given by the engine
	GLuint frameBufferID;		// Our handle to the Frame Buffer
	GLuint renderBufferID;		// Our handle to the Render Buffer
	GLuint *colorBufferID;		// Our handle to the Color buffer(s) attachment(s)

	Fbo();
	virtual ~Fbo();

	bool upload(string EngineFormat, int index, int Width, int Height, int iFormat, int Format, int Type, unsigned int numColorAttachments);
	void bind() const;
	void bind_tex(GLuint attachment = 0) const;
	void active(int index = 0) const;
				
private:


};

#endif