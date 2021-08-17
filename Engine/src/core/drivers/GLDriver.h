// glDriver.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/drivers/ImGuiDriver.h"
#include "core/renderer/Viewport.h"
#include "core/renderer/FboManager.h"

namespace Phoenix {

	// HACK: get rid of macros
#define GLDRV (&Phoenix::glDriver::getInstance())
#define GLDRV_MAX_COLOR_ATTACHMENTS 4

// ******************************************************************

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
		static glDriver& getInstance();
		static void release();

		tGLConfig		config;
		FboConfig		fboConfig[FBO_BUFFERS];
		tExprTkViewport	m_exprtkCurrentViewport; // Viewport variables for ExprTk
		float			m_mouseX, m_mouseY;

	private:
		DemoKernel&		m_demo;
		GLFWwindow*		m_glfw_window;
		Viewport		m_current_viewport;
		ImGuiDriver*	m_imGui;
		float			m_timeCurrentFrame;
		float			m_timeLastFrame;
		float			m_timeDelta;
		float			m_mouse_lastxpos, m_mouse_lastypos;
		bool			m_windowResizing;

	public:
		glDriver();

	private:
		// Callbacks
		static void glDebugMessage_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
		static void glfwErrorCallback(int, const char* err_str);
		static void glfwKeyCallback(GLFWwindow* p_glfw_window, int key, int scancode, int action, int mods);
		static void glfwMouseScrollCallback(GLFWwindow* p_glfw_window, double xoffset, double yoffset);
		static void glMouseMoveCallback(GLFWwindow* p_glfw_window, double xpos, double ypos);
		static void glfwMouseButtonCallback(GLFWwindow* p_glfw_window, int button, int action, int mods);
		static void glfwWindowSizeCallback(GLFWwindow* p_glfw_window, int width, int height);
		void OnWindowSizeChanged(GLFWwindow* p_glfw_window, int width, int height);

		void	initFbos();
		void	initStates();

		bool	checkGLError(char* pOut);
		
	public:
		void	initFramework();
		bool	initGraphics();
		void	initRender(int clear);
		void	drawGrid(bool drawAxisX, bool drawAxisY, bool drawAxisZ);
		void	drawGui();
		void	guiDrawLog();
		void	guiDrawInfo();
		void	guiDrawFpsHistogram();
		void	guiDrawVersion();
		void	guiDrawFbo();
		void	guiDrawSections();
		void	guiDrawSound();
		void	guiDrawGridPanel();
		void	guiDrawHelpPanel();
		void	guiChangeAttachment();
		void	guiAddLog(std::string message);

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
		void	moveWindow(int x, int y);
		void	resizeWindow(int width, int height);
		void	ProcessInput();

		void	swapBuffers();

		int		WindowShouldClose();
		void	close();

	private:

		
	};
}