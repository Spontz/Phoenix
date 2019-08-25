// gldriver.h
// Spontz Demogroup

#ifndef GLDRIVER_H
#define GLDRIVER_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define  GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/shader.h"
#include "core/texture.h"
#include "core/model.h"
#include "core/skinnedmesh.hpp"

// HACK: get rid of macros
#define GLDRV (&glDriver::GetInstance())
#define GLDRV_MAX_COLOR_ATTACHMENTS 4

// ******************************************************************

typedef struct {
	float width, height;
	int tex_iformat;
	int tex_format;
	int tex_type;
	int tex_components;
	int ratio;
	char* format;
	int numColorAttachments;
} tGLFboFormat;

// ******************************************************************

struct Viewport {
	Viewport() = default;
	Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

	// Enforce aspect, center viewport, keep maximum area (top/down or left/right "black bars")
	// given a render target size and a desired viewport aspect ratio.
	static Viewport FromRenderTarget(GLsizei rt_width, GLsizei rt_height, GLsizei vp_aspect_x, GLsizei vp_aspect_y);

	float GetAspectRatio() const { return static_cast<float>(width) / static_cast<float>(height);}

	GLint	x;
	GLint	y;
	GLsizei	width;
	GLsizei	height;
};

// ******************************************************************

void window_size_callback(GLFWwindow* window, int width, int height); // hack bind to static

class glDriver {

	friend class mathDriver; // for exprtk__ members
	friend struct InitScriptCommands; // for script__ members;
	friend void ::window_size_callback(GLFWwindow*, int, int); // hack bind to static

public:
	int				fullScreen;
	int				saveInfo;

private:
	int				script__gl_width__framebuffer_width_;
	int				script__gl_height__framebuffer_height_;
	float			script__gl_aspect__current_viewport_aspect_;

private:
	float			exprtk__vpWidth__current_viewport_width_;
	float			exprtk__vpHeight__current_viewport_height_;
	float			exprtk__aspectRatio__current_viewport_aspect_;

public:
	int				stencil;
	int				accum;
	int				multisampling;
	float			gamma;
	float			TimeCurrentFrame;
	float			TimeLastFrame;
	float			TimeDelta;
	tGLFboFormat	fbo[FBO_BUFFERS];

	float			mouse_lastxpos, mouse_lastypos;

public:
	glDriver();

public:
	static glDriver& GetInstance();

public:
	void initFramework();
	void initGraphics();
	void initStates();
	void initRender(int clear);
	void OnFramebufferSizeChanged();
	void setFramebuffer(); // Unbinds any framebuffer and sets default viewport
	void initFbos();
	void swapBuffers();
	void close();

	void drawFps();
	void drawTiming();
	void drawFbo();
	void processInput();

	int window_should_close();

	bool checkGLError(char* pOut);

	int getTextureFormatByName(char* name);
	int getTextureInternalFormatByName(char* name);
	int getTextureTypeByName(char* name);
	int getTextureComponentsByName(char* name);

	void			SetCurrentViewport(Viewport const& viewport);
	Viewport		GetFramebufferViewport() const {
		return Viewport::FromRenderTarget(
			script__gl_width__framebuffer_width_,
			script__gl_height__framebuffer_height_,
			framebuffer_aspect_x_,
			framebuffer_aspect_y_
		);
	};
	Viewport const&	GetCurrentViewport() const { return current_viewport_; };

private:
	GLFWwindow*	window;
	Viewport	current_viewport_;
	GLsizei		framebuffer_aspect_x_;
	GLsizei		framebuffer_aspect_y_;

	// Current rendertarget size
	GLsizei		current_rt_width_;
	GLsizei		current_rt_height_;
};

void window_size_callback(GLFWwindow* window, int width, int height);

#endif
