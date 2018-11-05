// gldriver.cpp
// Spontz Demogroup

#include "main.h"

using namespace std;
using namespace glm;

// Initialize the glDriver main pointer to NULL
glDriver* glDriver::m_pThis = NULL;

void window_size_callback(GLFWwindow * window, int width, int height) {
	glViewport(0, 0, width, height);
	if (width>=1 && height>=1) {
		GLDRV->projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(width), float(height), 0.1f, 10.0f);
		if (GLDRV->shader != nullptr) {
			GLDRV->shader->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);
		}
	}
}


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
//	for (int i = 0; i < FBO_BUFFERS; i++) {
//		glDriver.fbo[i].width = glDriver.fbo[i].height = glDriver.fbo[i].ratio = 0;
//	}
}

void glDriver::init() {
	// Initialize the library
	if (!glfwInit())
		return;

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

	glfwSetWindowSizeCallback(window, window_size_callback);

	// Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Initialize BASS
//	if (!BASS_Init(-1, 44100, 0, 0, NULL))
//		std::cout << "Failed to initialize BASS" << std::endl;

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

	//mesh = new Model( "data/models/alliance.obj");
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

	// Load music
	//D:/xphere/Spontz/Phoenix2/res/music/biomechanical.mp3
	//str = BASS_StreamCreateFile(FALSE, "../res/music/biomechanical.mp3", 0, 0, BASS_STREAM_PRESCAN);
	//if (str == 0)
	//	std::cout << "BASS cannot read file" << std::endl;
	//BASS_ChannelPlay(str, FALSE);
	//BASS_Start();
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
