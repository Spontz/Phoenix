// gldriver.h
// Spontz Demogroup

#ifndef GLDRIVER_H
#define GLDRIVER_H

#include <GLFW/glfw3.h>

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

private:
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

public:
	// TODO: make private
	int				stencil;
	int				accum;
	int				multisampling;
	int				vsync;
	float			gamma;
	float			TimeCurrentFrame;
	float			TimeLastFrame;
	float			TimeDelta;
	tGLFboFormat	fbo[FBO_BUFFERS];
	float			mouse_lastxpos, mouse_lastypos;

private:
	glDriver();

public:
	static glDriver& GetInstance();

private:
	static void GLFWWindowSizeCallback(GLFWwindow* p_glfw_window, int width, int height);
	void OnWindowSizeChanged(GLFWwindow* p_glfw_window, int width, int height);

public:
	void initFramework();
	void initGraphics();
	void initRender(int clear);

	void drawSceneInfo();
	void drawFps();
	void drawTiming();
	void drawFbo();


	void close();

	Viewport GetFramebufferViewport() const;
	float GetFramebufferAspectRatio() const;
	void SetFramebuffer(); // Unbinds any framebuffer and sets default viewport

	Viewport const& GetCurrentViewport() const;
	void SetCurrentViewport(Viewport const& viewport);

	void swapBuffers();
	void ProcessInput();
	int WindowShouldClose();

private:
	void initFbos();
	void initStates();
	
	bool checkGLError(char* pOut);

	int getTextureFormatByName(char* name);
	int getTextureInternalFormatByName(char* name);
	int getTextureTypeByName(char* name);
	int getTextureComponentsByName(char* name);
};

#endif
