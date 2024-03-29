// Fbo.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include <string>

namespace Phoenix {

	class Fbo final {
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