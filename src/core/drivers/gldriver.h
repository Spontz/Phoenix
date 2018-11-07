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

#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "rendering/Model.h"

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
	tGLFboFormat	fbo[FBO_BUFFERS];



	static glDriver* getInstance();
	glDriver();
	void initFramework();
	void initGraphics();
	void render(float time);
	void swap_buffers();
	void close();

	int window_should_close();

	// Matrices
	glm::vec3 cam_position;
	glm::vec3 cam_look_at;
	glm::vec3 cam_up;

	glm::mat4 world_matrix;
	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;
	
	// TODO: TO DELETE
	Model   * mesh;
	Shader  * shader;
	Texture * texture;

private:
	GLFWwindow* window;

	static glDriver* m_pThis;
	void loadcontent(); // TODO: Guarrada TO DELETE


};

void window_size_callback(GLFWwindow* window, int width, int height);

#endif