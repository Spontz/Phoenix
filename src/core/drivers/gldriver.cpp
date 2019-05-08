// gldriver.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/events.h"

using namespace std;
using namespace glm;

// Initialize the glDriver main pointer to NULL
glDriver* glDriver::m_pThis = NULL;

// **************************************************
// Demo states to show in the drawTiming information

char *stateStr[] = {"play", "play - RW", "play - FF", 
					"paused", "paused - RW", "paused - FF" };

// ******************************************************************

typedef struct {
	char *name;
	int tex_iformat;		// internalformat
	int tex_format;
	int tex_type;
	int tex_components;
} glTexTable_t;

// ******************************************************************

glTexTable_t textureModes[] = {
	{ "RGB",			GL_RGB8,				GL_RGB,				GL_UNSIGNED_BYTE,	3},
	{ "RGBA",			GL_RGBA8,				GL_RGBA,			GL_UNSIGNED_BYTE,	4},
	{ "RGB_16F",		GL_RGB16F,				GL_RGB,				GL_FLOAT,			3},
	{ "RGBA_16F",		GL_RGBA16F,				GL_RGBA,			GL_FLOAT,			4},
	{ "RGB_32F",		GL_RGB32F,				GL_RGB,				GL_FLOAT,			3},
	{ "RGBA_32F",		GL_RGBA32F,				GL_RGBA,			GL_FLOAT,			4},
};
#define TEXTURE_MODE (sizeof(textureModes) / sizeof(glTexTable_t))

// ******************************************************************



// CALBACKS **************************************************

void error_callback(int, const char* err_str)
{
	LOG->Error("GLFW Error: %s", err_str);
}

void window_size_callback(GLFWwindow * window, int width, int height) {
	GLDRV->width = width;
	GLDRV->height = height;
	if (GLDRV->width == 0)
		GLDRV->width = 1;
	if (GLDRV->height == 0)
		GLDRV->height = 1;

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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	DEMO->camera->ProcessMouseScroll((float)yoffset);
}

void glErrorCallback(GLenum source, GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	LOG->Error("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
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

// ----------------------------------------------------------------------

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
	int i = 0;
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
	
	for (i=0; i<FBO_BUFFERS; i++) {
		this->fbo[i].width = this->fbo[i].height = this->fbo[i].ratio = 0;
	}
	// Register error callback first
	glfwSetErrorCallback(error_callback);
}

void glDriver::initFramework() {
	// Initialize the library
	if (!glfwInit())
		LOG->Error("GL Framework could not be initialized!");
}

void glDriver::initGraphics() {
	int i;

	// Create a windowed mode window and its OpenGL context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (GLDRV->fullScreen)
		window = glfwCreateWindow(width, height, DEMO->demoName, glfwGetPrimaryMonitor(), nullptr);
	else
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

	// During init, enable debug output
	if (DEMO->debug) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glErrorCallback, 0);
		// If you want to disable all error messages, except the API error messages, then you have to disable all messages first and the enable explicitly the API error messages:
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
	}

	// init fbo's
	for (i = 0; i < FBO_BUFFERS; i++) {
		if (((this->fbo[i].width != 0) && (this->fbo[i].height != 0)) || (this->fbo[i].ratio != 0)) {
			if (this->fbo[i].ratio != 0) {
				this->fbo[i].width = (this->width / this->fbo[i].ratio);
				this->fbo[i].height = (this->height / this->fbo[i].ratio);
			}

			this->fbo[i].tex_iformat = getTextureInternalFormatByName(this->fbo[i].format);
			this->fbo[i].tex_format = getTextureFormatByName(this->fbo[i].format);
			this->fbo[i].tex_type = getTextureTypeByName(this->fbo[i].format);
			this->fbo[i].tex_components = getTextureComponentsByName(this->fbo[i].format);
			// Check if the format is valid
			if (this->fbo[i].tex_format > 0) {
				DEMO->fboManager.addFbo(this->fbo[i].width, this->fbo[i].height, this->fbo[i].tex_iformat, this->fbo[i].tex_format, this->fbo[i].tex_type, this->fbo[i].tex_components);
				LOG->Info(LOG_LOW, "Fbo %i uploaded: width: %i, height: %i, format: %s, components: %i, GLformat: %i, GLiformat: %i, GLtype: %i", i, this->fbo[i].width, this->fbo[i].height, this->fbo[i].format, this->fbo[i].tex_components, this->fbo[i].tex_format, this->fbo[i].tex_iformat, this->fbo[i].tex_type);
			}
			else {
				LOG->Error("Error in FBO definition: FBO number %i has a non recongised format: '%s', please check 'graphics.spo' file.", i, this->fbo[i].format);
			}
		}
	}

	// Init internal timer
	TimeCurrentFrame = static_cast<float>(glfwGetTime());
}

void glDriver::initStates()
{
	glDisable(GL_BLEND);						// blending disabled

	glDisable(GL_CULL_FACE);					// cull face disabled
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// draw cwise and ccwise in fill mode

	glEnable(GL_DEPTH_TEST);					// depth test enabled
	glDepthFunc(GL_LEQUAL);						// depth test comparison function set to LEQUAL
	// Enable multisampling
	if (this->multisampling)
		glEnable(GL_MULTISAMPLE);
}

void glDriver::initRender(int clear)
{
	// Vsync Management
	glfwSwapInterval(0); // 0 -Disabled, 1-60pfs, 2-30fps, 3-20fps,...

	// reset the default gl state
	this->initStates();
	
	// Set the internal timer
	TimeLastFrame = TimeCurrentFrame;
	TimeCurrentFrame = static_cast<float>(glfwGetTime());
	TimeDelta = TimeCurrentFrame - TimeLastFrame;

	// set the viewport to the correct size // TODO: S'ha de areglar el tema dels viewports....
	//glViewport(this->vpXOffset, this->vpYOffset, this->vpWidth, this->vpHeight);
	setViewport(0, 0, this->width, this->height);
	this->vpXOffset = 0;
	this->vpYOffset = 0;
	this->vpWidth = this->width;
	this->vpHeight = this->height;

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

void glDriver::setViewport(int x, int y, GLsizei width, GLsizei height)
{
	glViewport(0, 0, this->width, this->height);
}


int glDriver::window_should_close() {
	return glfwWindowShouldClose(window);
}

bool glDriver::checkGLError(char * pOut)
{
	GLenum err = glGetError();
	if (err == GL_NO_ERROR)
		return false;
	if (pOut) {
		switch (err) {
		case GL_INVALID_ENUM:					strcpy(pOut, (const char*)"INVALID_ENUM");					break;
		case GL_INVALID_VALUE:					strcpy(pOut, (const char*)"INVALID_VALUE");					break;
		case GL_INVALID_OPERATION:				strcpy(pOut, (const char*)"INVALID_OPERATION");				break;
		case GL_STACK_OVERFLOW:					strcpy(pOut, (const char*)"STACK_OVERFLOW");				break;
		case GL_STACK_UNDERFLOW:				strcpy(pOut, (const char*)"STACK_UNDERFLOW");				break;
		case GL_OUT_OF_MEMORY:					strcpy(pOut, (const char*)"INVALID_VALUE");					break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:	strcpy(pOut, (const char*)"INVALID_FRAMEBUFFER_OPERATION");	break;
		default:								strcpy(pOut, (const char*)"UNHANDLED ERROR");				break;
		}

	}
	return true;
}

void glDriver::swap_buffers() {
	glfwSwapBuffers(window);
}

void glDriver::close() {
	glfwSetWindowShouldClose(window, GL_TRUE);
	glfwTerminate();
}

void glDriver::drawFps() {
	DEMO->text->glPrintf(-1, 0.9f, "FPS: %.0f", DEMO->fps);

}


void glDriver::drawTiming() {
	char *state;
	if (DEMO->state & DEMO_PAUSE) {
		if (DEMO->state & DEMO_REWIND) state = stateStr[4];
		else if (DEMO->state & DEMO_FASTFORWARD) state = stateStr[5];
		else state = stateStr[3];

	}
	else {
		if (DEMO->state & DEMO_REWIND) state = stateStr[1];
		else if (DEMO->state & DEMO_FASTFORWARD) state = stateStr[2];
		else state = stateStr[0];
	}
	DEMO->text->glPrintf(-1, 0.8f, "%d - %.1f/%.1f", DEMO->frameCount, DEMO->runTime, DEMO->endTime);
	DEMO->text->glPrintf(-1, 0.7f, "sound %0.1f", BASSDRV->sound_cpu());
	DEMO->text->glPrintf(-1, 0.6f, "texmem %.2fM", DEMO->textureManager.mem);
	DEMO->text->glPrintf(-1, 0.5f, "%s", state);
}


int glDriver::getTextureFormatByName(char *name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_format;
		}
	}
	return -1;
}

int glDriver::getTextureInternalFormatByName(char *name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_iformat;
		}
	}
	return -1;
}

int glDriver::getTextureTypeByName(char *name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_type;
		}
	}
	return -1;
}

int glDriver::getTextureComponentsByName(char *name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_components;
		}
	}
	return -1;
}