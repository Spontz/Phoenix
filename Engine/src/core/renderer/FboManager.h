// fbo_manager.h
// Spontz Demogroup

#pragma once

#include <string>
#include <vector>
#include "core/renderer/Fbo.h"

namespace Phoenix {
	
	struct FboConfig {
		std::string	format;
		float		width = 0.0f;
		float		height = 0.0f;;
		int			ratio = 0;
		int			numColorAttachments = 0;
		bool		useBilinearFilter = true;	// Use Bilinear Filter?
	};

	class FboManager {
	public:
		std::vector<Fbo*>		fbo;		// Fbo's list
		float					mem;		// Fbo's memory in MegaBytes
		int						currentFbo;	// Current fboConfig (-1 if it's the frameBuffer)
		bool					clearColor;	// Clear color bit
		bool					clearDepth;	// Clear depth bit

		FboManager();
		void init();
		void bind(int fbo_num, bool clearColor, bool clearDepth);
		void bindCurrent();
		void bind_tex(int fbo_num, GLuint texUnit = 0, GLuint attachment = 0);
		void unbind(bool clearColor, bool clearDepth);
		int addFbo(const FboConfig& cfg);
		void clearFbos();
		int	getOpenGLTextureID(unsigned int index, unsigned int attachment = 0);

	private:
		int		getFormat(std::string const& name);
		int		getInternalFormat(std::string const& name);
		int		getType(std::string const& name);
		int		getComponents(std::string const& name);


		struct FboFormatTable {
			std::string	name;
			int			tex_iformat; // internalformat
			int			tex_format;
			int			tex_type;
			int			tex_components;
		};

		const std::vector<FboFormatTable> FboFormat = {
			{ "RGB",			GL_RGB8,				GL_RGB,				GL_UNSIGNED_BYTE,	3 },
			{ "RGBA",			GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,	4 },
			{ "RGB_16F",		GL_RGB16F,				GL_RGB,				GL_FLOAT,			3 },
			{ "RGBA_16F",		GL_RGBA16F,				GL_RGBA,			GL_FLOAT,			4 },
			{ "RGB_32F",		GL_RGB32F,				GL_RGB,				GL_FLOAT,			3 },
			{ "RGBA_32F",		GL_RGBA32F,				GL_RGBA,			GL_FLOAT,			4 },
			{ "RG_16F",			GL_RG16F,				GL_RG,				GL_FLOAT,			2 },
			{ "DEPTH",			GL_DEPTH_COMPONENT,		GL_DEPTH_COMPONENT,	GL_FLOAT,			1 },
			{ "DEPTH_16F",		GL_DEPTH_COMPONENT16,	GL_DEPTH_COMPONENT,	GL_FLOAT,			1 },
			{ "DEPTH_32F",		GL_DEPTH_COMPONENT32F,	GL_DEPTH_COMPONENT,	GL_FLOAT,			1 }
		};
	};
}