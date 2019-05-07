// fbo.cpp
// Spontz Demogroup

#include <main.h>
#include "fbo.h"

Fbo::Fbo(): use_linear(true), textureID(0)
{
	width = 0;
	height = 0;
	iformat = 0;
	format = 0;
	ttype = 0;
}

Fbo::~Fbo()
{
	//TODO: Fix! no es deleteTextures, em sembla...
	if (textureID != 0) {
		glDeleteTextures(1, &textureID);
		textureID = 0;
	}
}

bool Fbo::upload(int index, int Width, int Height, int iFormat, int Format, int Type)
{
	char	OGLError[1024];

	if ((Width == 0) || (Height == 0)) {
		LOG->Error("Fbo error: Size is zero!");
		return false;
	}
	this->width = Width;
	this->height= Height;
	this->iformat = iFormat;
	this->format = Format;
	this->ttype = Type;

	// Setup our FBO
	glGenFramebuffers(1, &(this->fboID));
	glBindFramebuffer(GL_FRAMEBUFFER, this->fboID);

	// Create the render buffer for depth	
	glGenRenderbuffers(1, &(this->depthBufferID));
	glBindRenderbuffer(GL_RENDERBUFFER, this->depthBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);

	// Setup a texture to render to
	glGenTextures(1, &(this->textureID));
	glBindTexture(GL_TEXTURE_2D, this->textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, this->iformat, this->width, this->height, 0, this->format, this->ttype, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Bind buffers
	glBindFramebuffer(GL_FRAMEBUFFER, this->fboID);
	glBindRenderbuffer(GL_RENDERBUFFER, this->depthBufferID);

	// And attach it to the FBO so we can render to it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureID, 0);
	// Attach the depth render buffer to the FBO as it's depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depthBufferID);

	if (GLDRV->checkGLError(OGLError))
		LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): %s", index, this->fboID, OGLError);

	// Check if any error during the framebuffer upload
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		switch (status) {
			case GL_FRAMEBUFFER_COMPLETE:
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): glCheckFramebufferStatus returned GL_FRAMEBUFFER_UNSUPPORTED. Choose other format, this is not supported in the current system.", index, this->fboID);
				break;
			default:
				LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): Invalid framebuffer status.", index, this->fboID);
				break;
		}
	}

	// Unbind buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// the binded texture has changed, todo: implement this in the manager?
	//fbo_reset_bind();

	if (GLDRV->checkGLError(OGLError)) {
		LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): %s", index, this->fboID, OGLError);
		return false;
	}
	
	return true;
}

void Fbo::bind() const
{
	// Bind buffers and make attachments
	glBindFramebuffer(GL_FRAMEBUFFER, this->fboID);
	glBindRenderbuffer(GL_RENDERBUFFER, this->depthBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureID, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depthBufferID);

	// Save the view port and set it to the size of the texture
	//glPushAttrib(GL_VIEWPORT_BIT);
	GLDRV->setViewport(0, 0, this->width, this->height);

}

void Fbo::bind_tex() const
{
	glBindTexture(GL_TEXTURE_2D, this->textureID);
	// TODO: Implement this in the manager
	//tex_reset_bind();	// Me must advise to the texture array that an fbo has been used 
	//fbo_current_tex = fbo->id_tex;
}

void Fbo::active(int index) const
{
	glActiveTexture(GL_TEXTURE0 + index);
}

