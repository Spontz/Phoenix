// fbo.cpp
// Spontz Demogroup

#include <main.h>
#include "fbo.h"

Fbo::Fbo(): use_linear(true), textureBufferID(0)
{
	width = 0;
	height = 0;
	iformat = 0;
	format = 0;
	ttype = 0;
}

Fbo::~Fbo()
{
	if (textureBufferID != 0) {
		glDeleteFramebuffers(1, &frameBufferID);
		glDeleteRenderbuffers(1, &renderBufferID);
		glDeleteTextures(1, &textureBufferID);
		textureBufferID = 0;
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

	// Setup our Framebuffer
	glGenFramebuffers(1, &(this->frameBufferID));
	glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferID);
	// Create a color attachment texture
	glGenTextures(1, &(this->textureBufferID));
	glBindTexture(GL_TEXTURE_2D, this->textureBufferID);
	glTexImage2D(GL_TEXTURE_2D, 0, this->iformat, this->width, this->height, 0, this->format, this->ttype, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureBufferID, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &(this->renderBufferID));
	glBindRenderbuffer(GL_RENDERBUFFER, this->renderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->width, this->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->renderBufferID);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->renderBufferID);

		

	if (GLDRV->checkGLError(OGLError))
		LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): %s", index, this->frameBufferID, OGLError);

	// Check if any error during the framebuffer upload
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		switch (status) {
		case GL_FRAMEBUFFER_COMPLETE:
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): glCheckFramebufferStatus returned GL_FRAMEBUFFER_UNSUPPORTED. Choose other format, this is not supported in the current system.", index, this->frameBufferID);
			break;
		default:
			LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): Invalid framebuffer status.", index, this->frameBufferID);
			break;
		}
	}

	// Unbind buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// the binded texture has changed, todo: implement this in the manager?
	//fbo_reset_bind();

	if (GLDRV->checkGLError(OGLError)) {
		LOG->Error("Fbo::upload: Error uploading fbo '%d' (id='%d'): %s", index, this->frameBufferID, OGLError);
		return false;
	}
	
	return true;
}

void Fbo::bind() const
{
	// Bind buffers and make attachments
	glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferID);
	glEnable(GL_DEPTH_TEST);
}

void Fbo::bind_tex() const
{
	glBindTexture(GL_TEXTURE_2D, this->textureBufferID);
}

void Fbo::active(int index) const
{
	glActiveTexture(GL_TEXTURE0 + index);
}

