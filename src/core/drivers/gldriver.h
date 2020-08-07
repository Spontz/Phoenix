// gldriver.h
// Spontz Demogroup

#ifndef GLDRIVER_H
#define GLDRIVER_H

#include <GLFW/glfw3.h>
#include "core/drivers/imGuidriver.h"
#include "core/viewport.h"



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
	char* format;
	int numColorAttachments;
};

// ******************************************************************

struct tGLConfig {
	int				fullScreen;
	unsigned int	framebuffer_width;
	unsigned int	framebuffer_height;
	float			framebuffer_aspect_ratio;
	int				stencil;
	int				multisampling;
	int				vsync;
};
// ******************************************************************

class glDriver {
	// hack, todo: remove friends
	friend class mathDriver; // for exprtk__ members
	friend struct InitScriptCommands; // for script__ members;

public:
	static glDriver& GetInstance();
	tGLFboFormat	fbo[FBO_BUFFERS];
	tGLConfig		config;
	float			m_mouseX, m_mouseY;

private:
	// hack: create exprtk_vars struct and pass to glDriver on construction
	float			exprtk__vpWidth__current_viewport_width_;
	float			exprtk__vpHeight__current_viewport_height_;
	float			exprtk__aspectRatio__current_viewport_aspect_ratio_;

	GLFWwindow*		m_glfw_window;
	Viewport		m_current_viewport;
	imGuiDriver*	m_imGui;
	float			m_timeCurrentFrame;
	float			m_timeLastFrame;
	float			m_timeDelta;
	float			m_mouse_lastxpos, m_mouse_lastypos;

private:
	glDriver();
	
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

	int		getTextureFormatByName(char* name);
	int		getTextureInternalFormatByName(char* name);
	int		getTextureTypeByName(char* name);
	int		getTextureComponentsByName(char* name);

public:
	void	initFramework();
	bool	initGraphics();
	void	initRender(int clear);
	void	drawGui();
	void	guiDrawFps();
	void	guiDrawFpsHistogram();
	void	guiDrawTiming();
	void	guiDrawVersion();
	void	guiDrawFbo();
	void	guiDrawSections();
	void	guiDrawSound();
	void	guiChangeAttachment();
	const std::string getVersion();
	const std::string getOpenGLVersion();

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
};

#endif
