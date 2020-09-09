// fbo.cpp
// Spontz Demogroup

#include <main.h>

#include "fbo.h"

Fbo::Fbo(): use_linear(true), m_colorAttachment(0)
{
	engineFormat = "FBO not inited";
	width = 0;
	height = 0;
	iformat = 0;
	format = 0;
	ttype = 0;
	numAttachments = 1; // By default, 1 attachment
}

Fbo::~Fbo()
{
	if (m_colorAttachment != nullptr) {
		glDeleteFramebuffers(1, &m_frameBuffer);
		glDeleteRenderbuffers(1, &m_depthAttachment);
		glDeleteTextures(this->numAttachments, m_colorAttachment);
		delete[] m_colorAttachment;
	}
}

bool Fbo::upload(std::string EngineFormat, int index, int Width, int Height, int iFormat, int Format, int Type, unsigned int numColorAttachments)
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
	this->numAttachments = numColorAttachments;

	// Check Color attachments
	if (this->numAttachments <= 0 ) {
		LOG->Error("Fbo::upload: Requested %d attachments for fbo_%d, but should be at least 1", this->numAttachments, index);
		this->numAttachments = 1;
	}
	if (this->numAttachments > GLDRV_MAX_COLOR_ATTACHMENTS) {
		LOG->Error("Fbo::upload: MAX number of attachments reached. Requested %d attachments for fbo_%d, but max attachments are: %d", this->numAttachments, index, GLDRV_MAX_COLOR_ATTACHMENTS);
		this->numAttachments = GLDRV_MAX_COLOR_ATTACHMENTS;
	}

	// Setup our Framebuffer
	glCreateFramebuffers(1, &m_frameBuffer);
	if (EngineFormat != "DEPTH") { // If its not a depth texture, means it's a color texture! :)
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
		// Create the color attachment(s) texture(s)
		this->m_colorAttachment = new GLuint[numAttachments];
		glCreateTextures(GL_TEXTURE_2D, numAttachments, m_colorAttachment);
		for (unsigned int i = 0; i < numAttachments; i++) {
			glBindTexture(GL_TEXTURE_2D, m_colorAttachment[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, ttype, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorAttachment[i], 0);
		}
		// create a depth buffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_depthAttachment);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment);
		
		//glCreateTextures(GL_TEXTURE_2D, 1, &m_depthAttachment);
		//glBindTexture(GL_TEXTURE_2D, this->m_depthAttachment);
		//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);

		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[GLDRV_MAX_COLOR_ATTACHMENTS];
		for (int i = 0; i < GLDRV_MAX_COLOR_ATTACHMENTS; i++) {
			attachments[i] = GL_COLOR_ATTACHMENT0 + i;
		}

		glDrawBuffers(this->numAttachments, attachments);
	}
	else {	// If it's a Depth texture...
		// create depth texture
		this->m_colorAttachment = new GLuint[this->numAttachments];
		glCreateTextures(GL_TEXTURE_2D, 1, m_colorAttachment);
		for (unsigned int i = 0; i < numAttachments; i++) {
			glBindTexture(GL_TEXTURE_2D, m_colorAttachment[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, ttype, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach depth texture as FBO's depth buffer
			glBindFramebuffer(GL_FRAMEBUFFER, (this->m_frameBuffer));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->m_colorAttachment[i], 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
	}

	// Check if any error during the framebuffer upload
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		switch (status) {
		case GL_FRAMEBUFFER_UNSUPPORTED:
			LOG->Error("Fbo::upload: Error uploading %s fbo '%d' (id='%d'): glCheckFramebufferStatus returned GL_FRAMEBUFFER_UNSUPPORTED. Choose other format, this is not supported in the current system.", this->engineFormat.c_str(), index, this->m_frameBuffer);
			break;
		default:
			LOG->Error("Fbo::upload: Error uploading %s fbo '%d' (id='%d'): Invalid framebuffer status.", this->engineFormat.c_str(), index, this->m_frameBuffer);
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
	glBindFramebuffer(GL_FRAMEBUFFER, this->m_frameBuffer);
}

void Fbo::bind_tex(GLuint TexUnit, GLuint attachment) const
{
	if (attachment < this->numAttachments)
		glBindTextureUnit(TexUnit, this->m_colorAttachment[attachment]);
}

