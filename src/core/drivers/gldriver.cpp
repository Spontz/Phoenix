// gldriver.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/events.h"

using namespace std;
using namespace glm;

// Initialize the glDriver main pointer to NULL
glDriver* glDriver::m_pThis = NULL;

// **************************************************

void window_size_callback(GLFWwindow * window, int width, int height) {
	glViewport(0, 0, width, height);
	if (width>=1 && height>=1) {
		GLDRV->projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(width), float(height), 0.1f, 10.0f);
		if (GLDRV->shader != nullptr) {
			GLDRV->shader->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);
		}
	}
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
				LOG->Info("Demo time: %.4f", DEMO->runTime);
				break;
			case KEY_PLAY_PAUSE:
				if (DEMO->state == DEMO_PLAY) {
					//DEMO->pause();//TODO
				}
				else {
					//DEMO->play();//TODO
				}
				break;
			case KEY_REWIND:
				//DEMO->rewind();//TODO
				break;
			case KEY_FASTFORWARD:
				//DEMO->fastforward();//TODO
				break;
			case KEY_RESTART:
				//DEMO->restart();//TODO
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
				DEMO->pause();
			}
			else {
				DEMO->play();
			}
			break;
		}
	
	}
/*	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		LOG->Info("Exit key pressed, closing demo...");
		DEMO->closeDemo();
		exit(EXIT_SUCCESS);
	}
	*/
	
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
	
	// Init matrices
	cam_position = glm::vec3(0.0f, 1.0f, 1.2f);
	cam_look_at = glm::vec3(0.0f, 0.5f, 0.0f);
	cam_up = glm::vec3(0.0f, 1.0f, 0.0f);

	world_matrix = glm::mat4(1.0f);
	view_matrix = glm::lookAt(cam_position, cam_look_at, cam_up);
	projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(width), float(height), 0.1f, 10.0f);

	// TODELETE
	mesh = nullptr;
	shader = nullptr;
	texture = nullptr;
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

	// Load content
	loadcontent();

	// Set the viewport
	glClearColor(0.6784f, 0.8f, 1.0f, 1.0f);
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
}


int glDriver::window_should_close() {
	return glfwWindowShouldClose(window);
}

void glDriver::loadcontent() {
	std::string s_demo = DEMO->demoDir;
	std::string s_file = s_demo + "/models/alliance.obj";

	mesh = new Model(s_file);

	/* Create and apply basic shader */
	shader = new Shader("Basic.vert", "Basic.frag");
	shader->apply();

	shader->setUniformMatrix4fv("world", world_matrix);
	shader->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(world_matrix))));
	shader->setUniformMatrix4fv("viewProj", projection_matrix * view_matrix);

	shader->setUniform3fv("cam_pos", cam_position);

	texture = new Texture();
	s_file = s_demo + "/models/alliance.png";
	texture->load(s_file);
	texture->bind();
}

void glDriver::render(float time) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Draw our triangle */
	world_matrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(-90.0f), glm::vec3(0, 1, 0));

	shader->setUniformMatrix4fv("world", world_matrix);
	shader->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(world_matrix))));

	shader->apply();
	texture->bind();
	mesh->Draw();
}

void glDriver::swap_buffers() {
	glfwSwapBuffers(window);
}

void glDriver::close() {
	glfwTerminate();
}
