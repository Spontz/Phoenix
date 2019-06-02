// fbo.cpp
// Spontz Demogroup

#include <main.h>
#include "fbo.h"

Fbo::Fbo(): use_linear(true), textureBufferID(0)
{
	engineFormat = "FBO not inited";
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

bool Fbo::upload(string EngineFormat, int index, int Width, int Height, int iFormat, int Format, int Type)
{
	if ((Width == 0) || (Height == 0)) {
		LOG->Error("Fbo error: Size is zero!");
		return false;
	}
	this->engineFormat = EngineFormat;
	this->width = Width;
	this->height= Height;
	this->iformat = iFormat;
	this->format = Format;
	this->ttype = Type;

	// Setup our Framebuffer
	glGenFramebuffers(1, &(this->frameBufferID));
	if (EngineFormat != "DEPTH") { // If its not a depth texture, means it's a color texture! :)
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
	}
	else {	// If it's a Depth texture...
		// create depth texture
		glGenTextures(1, &(this->textureBufferID));
		glBindTexture(GL_TEXTURE_2D, (this->textureBufferID));
		glTexImage2D(GL_TEXTURE_2D, 0, this->iformat, this->width, this->height, 0, this->format, this->ttype, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, (this->frameBufferID));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, (this->textureBufferID), 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	// Check if any error during the framebuffer upload
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		switch (status) {
		case GL_FRAMEBUFFER_UNSUPPORTED:
			LOG->Error("Fbo::upload: Error uploading %s fbo '%d' (id='%d'): glCheckFramebufferStatus returned GL_FRAMEBUFFER_UNSUPPORTED. Choose other format, this is not supported in the current system.", this->engineFormat.c_str(), index, this->frameBufferID);
			break;
		default:
			LOG->Error("Fbo::upload: Error uploading %s fbo '%d' (id='%d'): Invalid framebuffer status.", this->engineFormat.c_str(), index, this->frameBufferID);
			break;
		}
		return false;
	}

	// Unbind buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	return true;
}

void Fbo::bind() const
{
	// Bind buffers and make attachments
	glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferID);
}

void Fbo::bind_tex() const
{
	glBindTexture(GL_TEXTURE_2D, this->textureBufferID);
}

void Fbo::active(int index) const
{
	glActiveTexture(GL_TEXTURE0 + index);
}

