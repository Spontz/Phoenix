// gldriver.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/events.h"

using namespace std;
using namespace glm;

// Initialize the glDriver main pointer to NULL
glDriver* glDriver::m_pThis = NULL;

// **************************************************

void error_callback(int, const char* err_str)
{
	LOG->Error("GLFW Error: %s", err_str);
}
void window_size_callback(GLFWwindow * window, int width, int height) {
	GLDRV->width = width;
	GLDRV->height = height;
	GLDRV->initRender(true);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case KEY_EXIT:
			DEMO->exitDemo = true;
			break;
		case KEY_SCREENSHOT:
			//GLDRV->screenshot(); //TODO
			break;
		}
		if (DEMO->debug) {
			switch (key) {
			case KEY_TIME:
				LOG->Info(LOG_HIGH,"Demo time: %.4f", DEMO->runTime);
				break;
			case KEY_PLAY_PAUSE:
				if (DEMO->state == DEMO_PLAY) {
					DEMO->pauseDemo();
				}
				else {
					DEMO->playDemo();
				}
				break;
			case KEY_REWIND:
				DEMO->rewindDemo();
				break;
			case KEY_FASTFORWARD:
				DEMO->fastforwardDemo();
				break;
			case KEY_RESTART:
				DEMO->restartDemo();
				break;
			case KEY_SHOWTIME:
				if (DEMO->drawTiming)
					DEMO->drawTiming = 0;
				else
					DEMO->drawTiming = 1;
				break;
			case KEY_SHOWFPS:
				if (DEMO->drawFps)
					DEMO->drawFps = 0;
				else
					DEMO->drawFps = 1;
				break;
			case KEY_SHOWSOUND:
				if (DEMO->drawSound)
					DEMO->drawSound = 0;
				else
					DEMO->drawSound = 1;
				break;
			}
		}
		break;
	case GLFW_RELEASE:
		switch (key) {
		case KEY_REWIND:
		case KEY_FASTFORWARD:
			if (DEMO->state & DEMO_PAUSE) {
				DEMO->pauseDemo();
			}
			else {
				DEMO->playDemo();
			}
			break;
		}
	
	}
}

// **************************************************

glDriver * glDriver::getInstance() {
	if (m_pThis == NULL)
		m_pThis = new glDriver();
	return m_pThis;
}

glDriver::glDriver() {
	width = 200;
	height = 100;
	fullScreen = 0;
	saveInfo = 0;
	AspectRatio = 0;
	bpp = 32;
	zbuffer = 16;
	stencil = 0;
	accum = 0;
	multisampling = 0;
	gamma = 1.0f;
	
	// Register error callback first
	glfwSetErrorCallback(error_callback);
}

void glDriver::initFramework() {
	// Initialize the library
	if (!glfwInit())
		LOG->Error("GL Framework could not be initialized!");
}

void glDriver::initGraphics() {

	// Create a windowed mode window and its OpenGL context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, DEMO->demoName, nullptr, nullptr);

	if (!window) {
		glfwTerminate();
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Configure GLFW callbacks
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);

	// Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		LOG->Error("Failed to initialize GLAD");
	}

	// Init render
	this->initRender(true);
}

void glDriver::initStates()
{
	glDisable(GL_BLEND);						// blending disabled

	glDisable(GL_CULL_FACE);					// cull face disabled
	glEnable(GL_TEXTURE_2D);					// textures enabled
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// draw cwise and ccwise in fill mode

	glEnable(GL_DEPTH_TEST);					// depth test enabled
	glDepthFunc(GL_LEQUAL);						// depth test comparison function set to LEQUAL

	// Enable multisampling
	if (this->multisampling)
		glEnable(GL_MULTISAMPLE);
}

void glDriver::initRender(int clear)
{
	// reset the default gl state
	this->initStates();
	// set the viewport to the correct size // TODO: S'ha de areglar el tema dels viewports....
	//glViewport(this->vpXOffset, this->vpYOffset, this->vpWidth, this->vpHeight);
	glViewport(0, 0, this->width, this->height);

	// clear some buffers if needed
	if (clear) {
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

		if (this->stencil > 0) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
		else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}
}


int glDriver::window_should_close() {
	return glfwWindowShouldClose(window);
}

void glDriver::swap_buffers() {
	glfwSwapBuffers(window);
}

void glDriver::close() {
	glfwTerminate();
}
