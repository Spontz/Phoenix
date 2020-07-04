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

class glDriver {
	// hack, todo: remove friends
	friend class mathDriver; // for exprtk__ members
	friend struct InitScriptCommands; // for script__ members;

public:
	int				fullScreen;

public:
	// hack: create script_vars struct and pass to glDriver on construction
	unsigned int	script__gl_width__framebuffer_width_;
	unsigned int	script__gl_height__framebuffer_height_;
	float			script__gl_aspect__framebuffer_viewport_aspect_ratio_;

private:
	// hack: create exprtk_vars struct and pass to glDriver on construction
	float			exprtk__vpWidth__current_viewport_width_;
	float			exprtk__vpHeight__current_viewport_height_;
	float			exprtk__aspectRatio__current_viewport_aspect_ratio_;

private:
	GLFWwindow*		p_glfw_window_;
	Viewport		current_viewport_;
	GLsizei			current_rt_width_, current_rt_height_;
	imGuiDriver*	imGui_;

public:
	// TODO: make private
	int				stencil;
	int				multisampling;
	int				vsync;
	float			TimeCurrentFrame;
	float			TimeLastFrame;
	float			TimeDelta;
	tGLFboFormat	fbo[FBO_BUFFERS];
	float			mouse_lastxpos, mouse_lastypos;
	float			mouse_x, mouse_y;

private:
	glDriver();

public:
	static glDriver& GetInstance();

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

public:
	void	initFramework();
	bool	initGraphics();
	void	initRender(int clear);
	void	drawGui();
	void	guiDrawTiming();
	void	guiDrawFps();
	void	guiDrawSections();
	void	guiDrawFbo();
	void	guiChangeAttachment();

	void	close();

	Viewport		GetFramebufferViewport() const;
	float			GetFramebufferAspectRatio() const;
	void			SetFramebuffer(); // Unbinds any framebuffer and sets default viewport

	Viewport const& GetCurrentViewport() const;
	void			SetCurrentViewport(Viewport const& viewport);
	
	void	calcMousePos(float x, float y);

	void	swapBuffers();
	void	ProcessInput();
	int		WindowShouldClose();

private:
	void	initFbos();
	void	initStates();
	
	bool	checkGLError(char* pOut);

	int		getTextureFormatByName(char* name);
	int		getTextureInternalFormatByName(char* name);
	int		getTextureTypeByName(char* name);
	int		getTextureComponentsByName(char* name);
};

#endif
