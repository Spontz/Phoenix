// Fbo.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include <string>

namespace Phoenix {

	class Fbo final {

		// TODO: Crear una estructura con las carecteristicas de la fbo, como con la textura:
		// 
		// Especificar el Tipo de FBO: FRAMEBUFFER / DEPTH BUFFER / RENDERBUFFER --
		// Despu,es crear otro elemento que sea el FORMATO del buffer: RGB, RGBA, 8bits, etc...
		// Type of FBO Format Type supported
		/*
		enum class FboType {
			UNDEFINED,
			FRAMEBUFFER,
			DEPTHBUFFER,
			RENDERBUFFER
		};

		// TODO, USE MAPS?
		const std::map<std::string, FboType> fboBufferType = {
			{"undefined",		FboType::UNDEFINED},
			{"COLOR",			FboType::FRAMEBUFFER},
			{"DEPTH",			FboType::DEPTHBUFFER},
			{"RENDER",			FboType::RENDERBUFFER},
		};

	public:
		struct Properties final {
			FboType				m_bufferType;		// buffer type: FRAMEBUFFER, DEPTH, RENDER, etc.
			std::string			m_bufferFormat;		// buffer format: RGB, RGBA, RGB_8, etc.
		};
		*/
	public:

		int				width, height;
		int				iformat, format, ttype, components;
		unsigned int	numAttachments;
		bool			m_useLinearFilter;
		std::string		engineFormat;			// format of the FBO, by a name given by the engine
		GLuint			m_frameBuffer;			// Our handle to the Frame Buffer
		GLuint			m_depthAttachment;		// Our handle to the Render Buffer
		GLuint*			m_colorAttachment;		// Our handle to the Color buffer(s) attachment(s)

		Fbo();
		~Fbo();

		bool upload(std::string EngineFormat, int Width, int Height, int iFormat, int Format, int Type, unsigned int numColorAttachments, bool useLinearFilter);
		void bind() const;
		void bind_tex(GLuint texUnit = 0, GLuint attachment = 0) const;
		void clearColor() const;

	private:


	};
}