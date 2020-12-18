// glDriver.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/drivers/imGuiDriver.h"
#include "core/renderer/Viewport.h"

namespace Phoenix {

	// HACK: get rid of macros
#define GLDRV (&glDriver::GetInstance())
#define GLDRV_MAX_COLOR_ATTACHMENTS 4

// ******************************************************************

	struct tGLFboFormat {
		float width, height;
		int tex_iformat;
		int tex_format;
		int tex_type;
		int tex_components;
		int ratio;
		std::string format;
		int numColorAttachments;
	};

	struct tGLConfig {
		bool			fullScreen;
		unsigned int	framebuffer_width;
		unsigned int	framebuffer_height;
		float			framebuffer_aspect_ratio;
		int				stencil;
		int				multisampling;
		int				vsync;
	};

	struct tExprTkViewport {
		float			width;
		float			height;
		float			aspect_ratio;
	};

	// ******************************************************************

	class glDriver {

	public:
		static glDriver& GetInstance();
		tGLFboFormat	fbo[FBO_BUFFERS];
		tGLConfig		config;
		tExprTkViewport	exprTk_current_viewport; // Viewport variables for ExprTk
		float			m_mouseX, m_mouseY;

	private:
		demokernel&		m_demo;
		GLFWwindow*		m_glfw_window;
		Viewport		m_current_viewport;
		imGuiDriver*	m_imGui;
		float			m_timeCurrentFrame;
		float			m_timeLastFrame;
		float			m_timeDelta;
		float			m_mouse_lastxpos, m_mouse_lastypos;

	public:
		glDriver();

	private:
		// Callbacks
		static void glDebugMessage_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
		static void glfwError_callback(int, const char* err_str);
		static void key_callback(GLFWwindow* p_glfw_window, int key, int scancode, int action, int mods);
		static void mouseScroll_callback(GLFWwindow* p_glfw_window, double xoffset, double yoffset);
		static void mouseMove_callback(GLFWwindow* p_glfw_window, double xpos, double ypos);
		static void mouseButton_callback(GLFWwindow* p_glfw_window, int button, int action, int mods);
		static void glfwWindowSize_callback(GLFWwindow* p_glfw_window, int width, int height);
		void OnWindowSizeChanged(GLFWwindow* p_glfw_window, int width, int height);

		void	initFbos();
		void	initStates();

		bool	checkGLError(char* pOut);

		int		getTextureFormatByName(std::string const& name);
		int		getTextureInternalFormatByName(std::string const& name);
		int		getTextureTypeByName(std::string const& name);
		int		getTextureComponentsByName(std::string const& name);

	public:
		void	initFramework();
		bool	initGraphics();
		void	initRender(int clear);
		void	drawGrid();
		void	drawGui();
		void	guiDrawInfo();
		void	guiDrawFpsHistogram();
		void	guiDrawVersion();
		void	guiDrawFbo();
		void	guiDrawSections();
		void	guiDrawSound();
		void	guiDrawGridPanel();
		void	guiChangeAttachment();

		const std::string				getGLFWVersion();
		const std::string				getOpenGLVersion();
		const std::string				getOpenGLVendor();
		const std::string				getOpenGLRenderer();
		const std::vector<std::string>	getOpenGLExtensions();

		Viewport		GetFramebufferViewport() const;
		float			GetFramebufferAspectRatio() const;
		void			SetFramebuffer(); // Unbinds any framebuffer and sets default viewport

		Viewport const& GetCurrentViewport() const;
		void			SetCurrentViewport(Viewport const& viewport);

		void	calcMousePos(float x, float y);
		void	ProcessInput();

		void	swapBuffers();

		int		WindowShouldClose();
		void	close();

	private:

		struct tGLTextureTable {
			std::string	name;
			int			tex_iformat; // internalformat
			int			tex_format;
			int			tex_type;
			int			tex_components;
		};

		const std::vector<tGLTextureTable> textureModes = {
			{ "RGB",			GL_RGB8,				GL_RGB,				GL_UNSIGNED_BYTE,	3 },
			{ "RGBA",			GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,	4 },
			{ "RGB_16F",		GL_RGB16F,				GL_RGB,				GL_FLOAT,			3 },
			{ "RGBA_16F",		GL_RGBA16F,				GL_RGBA,			GL_FLOAT,			4 },
			{ "RGB_32F",		GL_RGB32F,				GL_RGB,				GL_FLOAT,			3 },
			{ "RGBA_32F",		GL_RGBA32F,				GL_RGBA,			GL_FLOAT,			4 },
			{ "RG_16F",			GL_RG16F,				GL_RG,				GL_FLOAT,			2 },
			{ "DEPTH",			GL_DEPTH_COMPONENT,		GL_DEPTH_COMPONENT,	GL_FLOAT,			1 }
		};
	};
}