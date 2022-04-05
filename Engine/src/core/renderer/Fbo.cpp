// Fbo.cpp
// Spontz Demogroup

#include <main.h>
#include "core/renderer/Fbo.h"

namespace Phoenix {

	Fbo::Fbo()
		:
		engineFormat("FBO not inited"),
		width(0),
		height(0),
		iformat(0),
		format(0),
		components(0),
		ttype(0),
		m_useLinearFilter(true),
		numAttachments(1),		// By default, 1 attachment
		m_frameBuffer(0),
		m_depthAttachment(0),
		m_colorAttachment(nullptr)
	{
	}

	Fbo::~Fbo()
	{
		if (m_frameBuffer) {
			glDeleteFramebuffers(1, &m_frameBuffer);
			glDeleteTextures(1, &m_depthAttachment);
			glDeleteTextures(numAttachments, m_colorAttachment);
			delete[] m_colorAttachment;
		}
	}

	bool Fbo::upload(std::string EngineFormat, int Width, int Height, int iFormat, int Format, int Type, unsigned int numColorAttachments, bool useLinearFilter)
	{
		if ((Width == 0) || (Height == 0)) {
			Logger::error("Fbo error: Size is zero!");
			return false;
		}
		engineFormat = EngineFormat;
		width = Width;
		height = Height;
		iformat = iFormat;
		format = Format;
		ttype = Type;
		numAttachments = numColorAttachments;
		m_useLinearFilter = useLinearFilter;

		// Check Color attachments
		if (numAttachments > FBO_MAX_COLOR_ATTACHMENTS) {
			Logger::error("Fbo::upload: MAX number of attachments reached. Requested {} attachments, but max attachments are: {}", numAttachments, FBO_MAX_COLOR_ATTACHMENTS);
			numAttachments = FBO_MAX_COLOR_ATTACHMENTS;
		}

		// Setup our Framebuffer
		glCreateFramebuffers(1, &m_frameBuffer);
		if (EngineFormat != "DEPTH") { // If its not a depth texture, means it's a color texture! :)
			glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
			// Create the color attachment(s) texture(s)
			m_colorAttachment = new GLuint[numAttachments];
			glCreateTextures(GL_TEXTURE_2D, numAttachments, m_colorAttachment);
			for (unsigned int i = 0; i < numAttachments; i++) {
				glBindTexture(GL_TEXTURE_2D, m_colorAttachment[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, ttype, NULL);
				if (m_useLinearFilter) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				else {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}
					
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorAttachment[i], 0);
			}
			// create a depth buffer object for depth and stencil attachment (we won't be sampling these)
			//glGenRenderbuffers(1, &m_depthAttachment);	// TODO: Replace this for glCreateRenderbuffers?? use glCreateTextures??
			//glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment);
			//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment);
			// Todo: Test if the depth attachment works properly
			glCreateTextures(GL_TEXTURE_2D, 1, &m_depthAttachment);
			glBindTexture(GL_TEXTURE_2D, m_depthAttachment);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);

			// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
			unsigned int attachments[FBO_MAX_COLOR_ATTACHMENTS];
			for (int i = 0; i < FBO_MAX_COLOR_ATTACHMENTS; i++) {
				attachments[i] = GL_COLOR_ATTACHMENT0 + i;
			}

			glDrawBuffers(numAttachments, attachments);
		}
		else {	// If it's a Depth texture...
			// create depth texture
			m_colorAttachment = new GLuint[numAttachments];
			glCreateTextures(GL_TEXTURE_2D, 1, m_colorAttachment);
			for (unsigned int i = 0; i < numAttachments; i++) {
				glBindTexture(GL_TEXTURE_2D, m_colorAttachment[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, ttype, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				// attach depth texture as FBO's depth buffer
				glBindFramebuffer(GL_FRAMEBUFFER, (m_frameBuffer));
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_colorAttachment[i], 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
		}

		// Check if any error during the framebuffer upload
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			switch (status) {
			case GL_FRAMEBUFFER_UNSUPPORTED:
				Logger::error("Fbo::upload: Error uploading {}: glCheckFramebufferStatus returned GL_FRAMEBUFFER_UNSUPPORTED. Choose other format, this is not supported in the current system.", engineFormat);
				break;
			default:
				Logger::error("Fbo::upload: Error uploading {}: Invalid framebuffer status.", engineFormat);
				break;
			}
			return false;
		}

		// Unbind buffers
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		return true;
	}

	void Fbo::bind() const
	{
		// Bind buffers and make attachments
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	}

	void Fbo::bind_tex(GLuint TexUnit, GLuint attachment) const
	{
		if (attachment < numAttachments)
			glBindTextureUnit(TexUnit, m_colorAttachment[attachment]);
	}

}