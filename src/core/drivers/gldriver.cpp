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

	GLDRV->mouse_lastxpos = (float)width / 2.0f;
	GLDRV->mouse_lastypos = (float)height / 2.0f;

	GLDRV->initRender(true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (DEMO->debug) {
		float x = (float)xpos;
		float y = (float)ypos;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			float xoffset = x - GLDRV->mouse_lastxpos;
			float yoffset = GLDRV->mouse_lastypos - y; // reversed since y-coordinates go from bottom to top

			GLDRV->mouse_lastxpos = x;
			GLDRV->mouse_lastypos = y;

			DEMO->camera->ProcessMouseMovement(xoffset, yoffset);
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			GLDRV->mouse_lastxpos = x;
			GLDRV->mouse_lastypos = y;
		}

	}
	
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	DEMO->camera->ProcessMouseScroll((float)yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS) {
		if (key == KEY_EXIT)
			DEMO->exitDemo = true;
		else if (key == KEY_SCREENSHOT) {
			//GLDRV->screenshot(); //TODO
		}
		if (DEMO->debug) {
			if (key == KEY_FASTFORWARD)
				DEMO->fastforwardDemo();
			else if (key == KEY_REWIND)
				DEMO->rewindDemo();
			else if (key == KEY_TIME)
				LOG->Info(LOG_HIGH, "Demo time: %.4f", DEMO->runTime);
			else if (key == KEY_PLAY_PAUSE) {
				if (DEMO->state == DEMO_PLAY)
					DEMO->pauseDemo();
				else
					DEMO->playDemo();
			}
			else if (key == KEY_RESTART)
				DEMO->restartDemo();
			else if (key == KEY_SHOWTIME)
				DEMO->drawTiming = !DEMO->drawTiming;
			else if (key == KEY_SHOWFPS)
				DEMO->drawFps = !DEMO->drawFps;
			else if (key == KEY_SHOWSOUND)
				DEMO->drawSound = !DEMO->drawSound;
			else if (key == KEY_CAPTURE) {
				DEMO->camera->CapturePos();
			}

		}
	}
	if (action == GLFW_RELEASE && DEMO->debug == TRUE) {
		if (key == KEY_FASTFORWARD || key == KEY_REWIND) {
			if (DEMO->state & DEMO_PAUSE)
				DEMO->pauseDemo();
			else
				DEMO->playDemo();
		}
	}
}

void glDriver::processInput()
{
	if (DEMO->debug) {
		if (glfwGetKey(window, KEY_FORWARD) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CAM_FORWARD, GLDRV->TimeDelta);
		if (glfwGetKey(window, KEY_BACKWARD) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CAM_BACKWARD, GLDRV->TimeDelta);
		if (glfwGetKey(window, KEY_STRAFELEFT) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CAM_LEFT, GLDRV->TimeDelta);
		if (glfwGetKey(window, KEY_STRAFERIGHT) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CAM_RIGHT, GLDRV->TimeDelta);
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
	mouse_lastxpos = width / 2.0f;
	mouse_lastypos = height / 2.0f;
	
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
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		LOG->Error("Failed to initialize GLAD");
	}

	// Init render
	this->initRender(true);

	// Init internal timer
	TimeCurrentFrame = static_cast<float>(glfwGetTime());
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
	
	// Set the internal timer
	TimeLastFrame = TimeCurrentFrame;
	TimeCurrentFrame = static_cast<float>(glfwGetTime());
	TimeDelta = TimeCurrentFrame - TimeLastFrame;

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


