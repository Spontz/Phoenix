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

#define GLDRV glDriver::getInstance()

using namespace std;

// ******************************************************************

typedef struct {
	int width, height;
	int tex_iformat;
	int tex_format;
	int tex_type;
	int ratio;
	char *format;
} tGLFboFormat;

// ******************************************************************

class glDriver {
	
public:
	float			AspectRatio;
	int				fullScreen;
	int				saveInfo;
	// Current rendertarget width and height
	int				width, height;
	// Current viewport (this data depends on: width, height and AspectRatio)
	int				vpWidth, vpHeight, vpYOffset, vpXOffset;
	int				bpp;
	int				zbuffer;
	int				stencil;
	int				accum;
	int				multisampling;
	float			gamma;
	float			TimeCurrentFrame;
	float			TimeLastFrame;
	float			TimeDelta;
	tGLFboFormat	fbo[FBO_BUFFERS];



	static glDriver* getInstance();
	glDriver();
	void initFramework();
	void initGraphics();
	void initStates();
	void initRender(int clear);
	void swap_buffers();
	void close();

	void processInput();

	int window_should_close();

private:
	GLFWwindow* window;

	static glDriver* m_pThis;


};

void window_size_callback(GLFWwindow* window, int width, int height);

#endif