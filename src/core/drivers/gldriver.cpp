// glDriver.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/Events.h"
#include "core/drivers/imGuiDriver.h"
#include "debug/Instrumentor.h"

// ******************************************************************

typedef struct {
	char* name;
	int tex_iformat; // internalformat
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
	{ "RG_16F",			GL_RG16F,				GL_RG,				GL_FLOAT,			2},
	{ "DEPTH",			GL_DEPTH_COMPONENT,		GL_DEPTH_COMPONENT,	GL_FLOAT,			1}
};
#define TEXTURE_MODE (sizeof(textureModes) / sizeof(glTexTable_t))

// GLFW CALLBACKS ***************************************************

void glDriver::glfwError_callback(int, const char* err_str)
{
	Logger::error("GLFW Error: %s", err_str);
}

void glDriver::glfwWindowSize_callback(GLFWwindow* p_glfw_window, int width, int height) {
	GLDRV->OnWindowSizeChanged(p_glfw_window, width, height);
}

void glDriver::OnWindowSizeChanged(GLFWwindow* p_glfw_window, int width, int height) {
	// Get a minimal window size
	width = std::max(width, 1);
	height = std::max(height, 1);

	config.framebuffer_width = width;
	config.framebuffer_height = height;
	
	// Change the debug font Size when we resize the screen
	m_imGui->changeFontSize(m_demo.m_debug_fontSize, width, height);

	// RT will be set to FB later
	//GLDRV->current_rt_width_ = width;
	//GLDRV->current_rt_height_ = height;

	m_mouse_lastxpos = static_cast<float>(width) / 2.0f;
	m_mouse_lastypos = static_cast<float>(height) / 2.0f;

	// Recalculate viewport sizes
	SetCurrentViewport(GetFramebufferViewport());

	// Recalculate fbo's with the new window size
	initFbos();

	initRender(true);
}

void glDriver::mouseMove_callback(GLFWwindow* p_glfw_window, double xpos, double ypos)
{
	if (GLDRV->m_demo.m_debug && !(ImGui::GetIO().WantCaptureMouse)) {
		float x = static_cast<float>(xpos);
		float y = static_cast<float>(ypos);

		// Move camera with Left click
		if (glfwGetMouseButton(p_glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			float xoffset = x - GLDRV->m_mouse_lastxpos;
			float yoffset = GLDRV->m_mouse_lastypos - y; // reversed since y-coordinates go from bottom to top

			GLDRV->m_mouse_lastxpos = x;
			GLDRV->m_mouse_lastypos = y;

			GLDRV->m_demo.m_pCamera->ProcessMouseMovement(xoffset, yoffset);
		}
		if (glfwGetMouseButton(p_glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			GLDRV->m_mouse_lastxpos = x;
			GLDRV->m_mouse_lastypos = y;
		}
		// Capture mouse position with Right click
		if (glfwGetMouseButton(p_glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			GLDRV->calcMousePos(x, y);
		}
		

	}
}

void glDriver::mouseButton_callback(GLFWwindow* p_glfw_window, int button, int action, int mods)
{
	if (GLDRV->m_demo.m_debug && !(ImGui::GetIO().WantCaptureMouse)) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			GLDRV->calcMousePos(GLDRV->m_mouse_lastxpos, GLDRV->m_mouse_lastypos);
			Logger::sendEditor("Mouse pos [%.4f, %.4f]", GLDRV->m_mouseX, GLDRV->m_mouseY);
		}
	}
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void glDriver::mouseScroll_callback(GLFWwindow* p_glfw_window, double xoffset, double yoffset)
{
	if (GLDRV->m_demo.m_debug && !(ImGui::GetIO().WantCaptureMouse))
		GLDRV->m_demo.m_pCamera->ProcessMouseScroll((float)yoffset);
}

void glDriver::glDebugMessage_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	Logger::info(LogLevel::low, "Error GL callback: %s type = 0x%x, severity = 0x%x, message = %s",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void glDriver::key_callback(GLFWwindow* p_glfw_window, int key, int scancode, int action, int mods) {
	
	demokernel& demo = GLDRV->m_demo;

	if (action == GLFW_PRESS) {
		if (key == KEY_EXIT)
			demo.m_exitDemo = true;
		if (demo.m_debug) {
			if (key == KEY_FASTFORWARD)
				demo.fastforwardDemo();
			else if (key == KEY_REWIND)
				demo.rewindDemo();
			else if (key == KEY_TIME)
				Logger::info(LogLevel::high, "Demo time: %.4f", demo.m_demoRunTime);
			else if (key == KEY_PLAY_PAUSE) {
				if (demo.m_status == DemoStatus::PLAY)
					demo.pauseDemo();
				else
					demo.playDemo();
			}
			else if (key == KEY_RESTART)
				demo.restartDemo();
			else if (key == KEY_SHOWINFO)
				GLDRV->guiDrawInfo();
			else if (key == KEY_SHOWVERSION)
				GLDRV->guiDrawVersion();
			else if (key == KEY_SHOWFPSHIST)
				GLDRV->guiDrawFpsHistogram();
			else if (key == KEY_SHOWFBO)
				GLDRV->guiDrawFbo();
			else if (key == KEY_CHANGEATTACH)
				GLDRV->guiChangeAttachment(); 
			else if (key == KEY_SHOWSECTIONINFO)
				GLDRV->guiDrawSections();
			else if (key == KEY_SHOWSOUND)
				GLDRV->guiDrawSound();
			
			

			else if (key == KEY_CAPTURE)
				demo.m_pCamera->CapturePos();
			else if (key == KEY_CAMRESET)
				demo.m_pCamera->Reset();
			else if (key == KEY_MULTIPLIER) 
				demo.m_pCamera->MovementSpeed *= 2.0f;
			else if (key == KEY_DIVIDER) {
				demo.m_pCamera->MovementSpeed /= 2.0f;
			}

		}
	}
	if (action == GLFW_RELEASE && demo.m_debug == TRUE) {
		if (key == KEY_FASTFORWARD || key == KEY_REWIND) {
			if (demo.m_status & DemoStatus::PAUSE)
				demo.pauseDemo();
			else
				demo.playDemo();
		}
	}
}

// glDriver *********************************************************

glDriver& glDriver::GetInstance() {
	static glDriver obj;
	return obj;
}

void glDriver::ProcessInput()
{	
	PX_PROFILE_FUNCTION();

	if (m_demo.m_debug) {
		if (glfwGetKey(m_glfw_window, KEY_FORWARD) == GLFW_PRESS)
			m_demo.m_pCamera->ProcessKeyboard(CameraMovement::FORWARD, GLDRV->m_timeDelta);
		if (glfwGetKey(m_glfw_window, KEY_BACKWARD) == GLFW_PRESS)
			m_demo.m_pCamera->ProcessKeyboard(CameraMovement::BACKWARD, GLDRV->m_timeDelta);
		if (glfwGetKey(m_glfw_window, KEY_STRAFELEFT) == GLFW_PRESS)
			m_demo.m_pCamera->ProcessKeyboard(CameraMovement::LEFT, GLDRV->m_timeDelta);
		if (glfwGetKey(m_glfw_window, KEY_STRAFERIGHT) == GLFW_PRESS)
			m_demo.m_pCamera->ProcessKeyboard(CameraMovement::RIGHT, GLDRV->m_timeDelta);
		if (glfwGetKey(m_glfw_window, KEY_ROLLRIGHT) == GLFW_PRESS)
			m_demo.m_pCamera->ProcessKeyboard(CameraMovement::ROLL_RIGHT, GLDRV->m_timeDelta);
		if (glfwGetKey(m_glfw_window, KEY_ROLLLEFT) == GLFW_PRESS)
			m_demo.m_pCamera->ProcessKeyboard(CameraMovement::ROLL_LEFT, GLDRV->m_timeDelta);
	}

}

glDriver::glDriver()
	:
	m_demo(demokernel::GetInstance()),
	m_timeCurrentFrame(0.0f),
	m_timeDelta(0.0f),
	m_timeLastFrame(0.0f),
	m_glfw_window(nullptr),
	m_imGui(nullptr),

	m_current_viewport{ 0,0,0,0 },

	m_mouse_lastxpos(0),
	m_mouse_lastypos(0),
	m_mouseX(0),
	m_mouseY(0),

	fbo{ tGLFboFormat{0.0f,0.0f,0,0,0,0,0,nullptr,0} },
	config { tGLConfig{ 0, 640, 480, (float)16.0f/9.0f, 0,0,0}},
	exprTk_current_viewport{ tExprTkViewport {0,0,0} }
{
	// Config
	config.framebuffer_width = 640;
	config.framebuffer_height = 480;
	config.framebuffer_aspect_ratio = UIntFraction{ 16,9 }.GetRatio();
	config.fullScreen = 0;
	config.stencil = 0;
	config.multisampling = 0;
	config.vsync = 0;
	
	for (auto i = 0; i < FBO_BUFFERS; ++i) {
		fbo[i].width = 0;
		fbo[i].height = 0;
		fbo[i].ratio = 0;
	}
}

void glDriver::initFramework() {
	// Initialize the library
	if (!glfwInit()) {
		Logger::error("GL Framework could not be initialized!");
		return;
	}
	else {
		glfwSetErrorCallback(glfwError_callback);
	}
}

bool glDriver::initGraphics() {
	m_mouse_lastxpos = config.framebuffer_width / 2.0f;
	m_mouse_lastypos = config.framebuffer_height / 2.0f;

	// Create a windowed mode window and its OpenGL context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_glfw_window = glfwCreateWindow(
		config.framebuffer_width,
		config.framebuffer_height,
		m_demo.m_demoName,
		config.fullScreen ? glfwGetPrimaryMonitor() : nullptr,
		nullptr
	);

	if (!m_glfw_window) {
		glfwTerminate();
		//Logger::error("Window was not created. OpenGL version not supported?");
		return false;
	}
		
	// Enable multisampling (aka anti-aliasing)
	if (config.multisampling)
		glfwWindowHint(GLFW_SAMPLES, 4); // This does mean that the size of all the buffers is increased by 4

	// Make the window's context current
	glfwMakeContextCurrent(m_glfw_window);

	// Configure GLFW callbacks
	glfwSetWindowSizeCallback(m_glfw_window, glfwWindowSize_callback);
	glfwSetKeyCallback(m_glfw_window, key_callback);
	glfwSetCursorPosCallback(m_glfw_window, mouseMove_callback);
	glfwSetMouseButtonCallback(m_glfw_window, mouseButton_callback);

	glfwSetScrollCallback(m_glfw_window, mouseScroll_callback);

	// Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwTerminate();
		Logger::error("Failed to initialize GLAD");
		return false;
	}
	
	// Enable multisampling state (aka anti-aliasing)
	if (config.multisampling)
		glEnable(GL_MULTISAMPLE);

	// Calculate the Viewport sizes
	SetCurrentViewport(GetFramebufferViewport());

	// Init render
	initRender(true);

	// imGUI init
	if (m_glfw_window) {
		m_imGui = new imGuiDriver();
		m_imGui->init(m_glfw_window);
		m_imGui->changeFontSize(m_demo.m_debug_fontSize, config.framebuffer_width, config.framebuffer_height);
	}
	

	// During init, enable debug output
	if (m_demo.m_debug) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugMessage_callback, 0);
		// If you want to disable all error messages, except the API error messages, then you have to disable all messages first and the enable explicitly the API error messages:
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
	}

	// init fbo's
	initFbos();

	// Init internal timer
	m_timeCurrentFrame = static_cast<float>(glfwGetTime());

	return true;
}

void glDriver::initStates() {
	glDisable(GL_BLEND);						// blending disabled
	glBlendFunc(GL_ONE, GL_ONE);				// Additive blending function by default
	glBlendEquation(GL_FUNC_ADD);				// ADD function by default

	glDisable(GL_CULL_FACE);					// cull face disabled
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// draw cwise and ccwise in fill mode

	glEnable(GL_DEPTH_TEST);					// depth test enabled
	glDepthFunc(GL_LEQUAL);						// depth test comparison function set to LEQUAL

	// Init lights colors, fbo's, shader ID's and texture States
	m_demo.m_lightManager.initAllLightsColors();
	m_demo.m_fboManager.unbind(true, true);
	m_demo.m_shaderManager.unbindShaders();
	m_demo.m_textureManager.initTextureStates();
}

void glDriver::initRender(int clear) {
	// Vsync Management
	glfwSwapInterval(config.vsync); // 0 -Disabled, 1-60pfs, 2-30fps, 3-20fps,...

	// reset the default gl state
	this->initStates();

	// Set the internal timer
	m_timeLastFrame = m_timeCurrentFrame;
	m_timeCurrentFrame = static_cast<float>(glfwGetTime());
	m_timeDelta = m_timeCurrentFrame - m_timeLastFrame;

	// set the viewport to the standard size
	// setViewport(vpXOffset, vpYOffset, static_cast<int>(vpWidth), static_cast<int>(vpHeight));
	SetCurrentViewport(GetFramebufferViewport());

	// clear some buffers if needed
	if (clear) {
		glClearColor(0, 0, 0, 0);

		if (config.stencil > 0) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
		else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}
}

void glDriver::drawGui()
{
	m_imGui->drawGui();
}

void glDriver::guiDrawVersion()
{
	m_imGui->show_version = !m_imGui->show_version;
}

void glDriver::guiDrawInfo()
{
	m_imGui->show_info = !m_imGui->show_info;
}

void glDriver::guiDrawFpsHistogram()
{
	m_imGui->show_fpsHistogram = !m_imGui->show_fpsHistogram;
}

void glDriver::guiDrawSections()
{
	m_imGui->show_sesctionInfo = !m_imGui->show_sesctionInfo;
}

void glDriver::guiDrawSound()
{
	m_imGui->show_sound = !m_imGui->show_sound;
}

void glDriver::guiDrawFbo()
{
	m_imGui->num_fboSetToDraw++;
	m_imGui->show_fbo = true;
	if (m_imGui->num_fboSetToDraw > (ceil((float)FBO_BUFFERS / (float)m_imGui->num_fboPerPage)))
	{
		m_imGui->num_fboSetToDraw = 0;
		m_imGui->show_fbo = false;
	}		
}

void glDriver::guiChangeAttachment()
{
	GLDRV->m_imGui->num_fboAttachmentToDraw++;
	if (GLDRV->m_imGui->num_fboAttachmentToDraw >= GLDRV_MAX_COLOR_ATTACHMENTS)
		GLDRV->m_imGui->num_fboAttachmentToDraw = 0;
}

Viewport glDriver::GetFramebufferViewport() const {
	return Viewport::FromRenderTargetAndAspectRatio(
		config.framebuffer_width,
		config.framebuffer_height,
		config.framebuffer_aspect_ratio
	);
}

Viewport const& glDriver::GetCurrentViewport() const {
	return m_current_viewport;
}

float glDriver::GetFramebufferAspectRatio() const {
	return static_cast<float>(config.framebuffer_width) / static_cast<float>(config.framebuffer_height);
}

void glDriver::SetCurrentViewport(Viewport const& viewport) {
	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

	m_current_viewport = viewport;

	exprTk_current_viewport.width = static_cast<float>(viewport.width);
	exprTk_current_viewport.height = static_cast<float>(viewport.height);
	exprTk_current_viewport.aspect_ratio = m_current_viewport.GetAspectRatio();
}

void glDriver::SetFramebuffer() {
	m_demo.m_fboManager.unbind(false, false);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Restore the driver viewport
	//SetCurrentViewport(GetFramebufferViewport());
}

void glDriver::initFbos() {
	////////////// efxBloom FBO Manager: internal FBO's that are being used by the engine effects
	{
		// Clear Fbo's, if there is any
		if (m_demo.m_efxBloomFbo.fbo.size() > 0) {
			Logger::info(LogLevel::low, "Ooops! we need to regenerate the Bloom efx FBO's! clearing efx FBO's first!");
			m_demo.m_efxBloomFbo.clearFbos();
		}

		// init fbo's for Bloom
		tGLFboFormat bloomFbo;
		bloomFbo.format = "RGB_16F";
		bloomFbo.numColorAttachments = 1;
		bloomFbo.ratio = 4;
		bloomFbo.width = static_cast<float>(config.framebuffer_width) / static_cast <float>(bloomFbo.ratio);
		bloomFbo.height = static_cast<float>(config.framebuffer_height) / static_cast <float>(bloomFbo.ratio);
		bloomFbo.tex_iformat = getTextureInternalFormatByName(bloomFbo.format);
		bloomFbo.tex_format = getTextureFormatByName(bloomFbo.format);
		bloomFbo.tex_type = getTextureTypeByName(bloomFbo.format);
		bloomFbo.tex_components = getTextureComponentsByName(bloomFbo.format);

		int res = 0;
		for (int i = 0; i < EFXBLOOM_FBO_BUFFERS; i++) {
			res = m_demo.m_efxBloomFbo.addFbo(bloomFbo.format, (int)bloomFbo.width, (int)bloomFbo.height, bloomFbo.tex_iformat, bloomFbo.tex_format, bloomFbo.tex_type, bloomFbo.tex_components, bloomFbo.numColorAttachments);
			if (res >= 0)
				Logger::info(LogLevel::low, "EfxBloom Fbo %i uploaded: width: %.0f, height: %.0f, format: %s, components: %i, GLformat: %i, GLiformat: %i, GLtype: %i", i, bloomFbo.width, bloomFbo.height, bloomFbo.format, bloomFbo.tex_components, bloomFbo.tex_format, bloomFbo.tex_iformat, bloomFbo.tex_type);
			else
				Logger::error("Error in efxBloom Fbo definition: Efx_Fbo number %i has a non recongised format: '%s', please blame the coder.", i, bloomFbo.format);
		}

	}

	////////////// efxAccum FBO Manager: internal FBO's that are being used by the engine effects
	// TODO: Que pasa si varios efectos de Accum se lanzan a la vez? no pueden usar la misma textura, asi que se mezclarán! deberíamos tener una fbo por cada efecto? es un LOCURON!!
	{
		// Clear Fbo's, if there is any
		if (m_demo.m_efxAccumFbo.fbo.size() > 0) {
			Logger::info(LogLevel::low, "Ooops! we need to regenerate the Accum efx FBO's! clearing efx FBO's first!");
			m_demo.m_efxAccumFbo.clearFbos();
		}

		// init fbo's for Accum
		tGLFboFormat accumFbo;
		accumFbo.format = "RGBA_16F";
		accumFbo.numColorAttachments = 1;
		accumFbo.ratio = 1;
		accumFbo.width = static_cast<float>(config.framebuffer_width) / static_cast <float>(accumFbo.ratio);
		accumFbo.height = static_cast<float>(config.framebuffer_height) / static_cast <float>(accumFbo.ratio);
		accumFbo.tex_iformat = getTextureInternalFormatByName(accumFbo.format);
		accumFbo.tex_format = getTextureFormatByName(accumFbo.format);
		accumFbo.tex_type = getTextureTypeByName(accumFbo.format);
		accumFbo.tex_components = getTextureComponentsByName(accumFbo.format);

		int res = 0;
		for (int i = 0; i < EFXACCUM_FBO_BUFFERS; i++) {
			res = m_demo.m_efxAccumFbo.addFbo(accumFbo.format, (int)accumFbo.width, (int)accumFbo.height, accumFbo.tex_iformat, accumFbo.tex_format, accumFbo.tex_type, accumFbo.tex_components, accumFbo.numColorAttachments);
			if (res >= 0)
				Logger::info(LogLevel::low, "EfxAccum Fbo %i uploaded: width: %.0f, height: %.0f, format: %s, components: %i, GLformat: %i, GLiformat: %i, GLtype: %i", i, accumFbo.width, accumFbo.height, accumFbo.format, accumFbo.tex_components, accumFbo.tex_format, accumFbo.tex_iformat, accumFbo.tex_type);
			else
				Logger::error("Error in efxAccum Fbo definition: Efx_Fbo number %i has a non recongised format: '%s', please blame the coder.", i, accumFbo.format);
		}

	}

	////////////// FBO Manager: Generic FBO's that can be used by the user
	// Clear Fbo's, if there is any
	if (m_demo.m_fboManager.fbo.size() > 0) {
		Logger::info(LogLevel::low, "Ooops! we need to regenerate the FBO's! clearing generic FBO's first!");
		m_demo.m_fboManager.clearFbos();
	}

	// init fbo's
	for (int i = 0; i < FBO_BUFFERS; i++) {
		if (((this->fbo[i].width != 0) && (this->fbo[i].height != 0)) || (this->fbo[i].ratio != 0)) {
			if (this->fbo[i].ratio != 0) {
				this->fbo[i].width = (float)(config.framebuffer_width / this->fbo[i].ratio);
				this->fbo[i].height = (float)(config.framebuffer_height / this->fbo[i].ratio);
			}

			this->fbo[i].tex_iformat = getTextureInternalFormatByName(this->fbo[i].format);
			this->fbo[i].tex_format = getTextureFormatByName(this->fbo[i].format);
			this->fbo[i].tex_type = getTextureTypeByName(this->fbo[i].format);
			this->fbo[i].tex_components = getTextureComponentsByName(this->fbo[i].format);
			// Check if the format is valid
			if (this->fbo[i].tex_format > 0) {
				m_demo.m_fboManager.addFbo(this->fbo[i].format, (int)this->fbo[i].width, (int)this->fbo[i].height, this->fbo[i].tex_iformat, this->fbo[i].tex_format, this->fbo[i].tex_type, this->fbo[i].tex_components, this->fbo[i].numColorAttachments);
				Logger::info(LogLevel::low, "Fbo %i uploaded: width: %.0f, height: %.0f, format: %s, components: %i, GLformat: %i, GLiformat: %i, GLtype: %i", i, this->fbo[i].width, this->fbo[i].height, this->fbo[i].format, this->fbo[i].tex_components, this->fbo[i].tex_format, this->fbo[i].tex_iformat, this->fbo[i].tex_type);
			}
			else {
				Logger::error("Error in FBO definition: FBO number %i has a non recongised format: '%s', please check 'graphics.spo' file.", i, this->fbo[i].format);
			}
		}
	}

}

int glDriver::WindowShouldClose() {
	return glfwWindowShouldClose(m_glfw_window);
}

bool glDriver::checkGLError(char* pOut) {
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
		case GL_OUT_OF_MEMORY:					strcpy(pOut, (const char*)"OUT_OF_MEMORY");					break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:	strcpy(pOut, (const char*)"INVALID_FRAMEBUFFER_OPERATION");	break;
		default:								strcpy(pOut, (const char*)"UNHANDLED ERROR");				break;
		}

	}
	return true;
}

void glDriver::swapBuffers() {
	glfwSwapBuffers(m_glfw_window);
}

void glDriver::close() {
	// Close ImGui
	m_imGui->close();
	
	// Close GLFW
	glfwSetWindowShouldClose(m_glfw_window, GL_TRUE);
	glfwTerminate();
}

const std::string glDriver::getGLFWVersion()
{
	return glfwGetVersionString();
}

const std::string glDriver::getOpenGLVersion()
{
	std::string strVersion = (const char*)glGetString(GL_VERSION);
	return strVersion;
}

const std::string glDriver::getOpenGLVendor()
{
	std::string strVersion = (const char*)glGetString(GL_VENDOR);
	return strVersion;
}

const std::string glDriver::getOpenGLRenderer()
{
	std::string strVersion = (const char*)glGetString(GL_RENDERER);
	return strVersion;
}

const std::vector<std::string> glDriver::getOpenGLExtensions()
{
	// Show supported OpenGL extensions
	std::vector<std::string> sv;
	
	GLint nExt = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &nExt);
	for (GLint i = 0; i < nExt; i++)
	{
		std::string s = (const char*)glGetStringi(GL_EXTENSIONS, i);
		sv.push_back(s);
	}
	
	return sv;
}

int glDriver::getTextureFormatByName(char* name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_format;
		}
	}
	return -1;
}

int glDriver::getTextureInternalFormatByName(char* name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_iformat;
		}
	}
	return -1;
}

int glDriver::getTextureTypeByName(char* name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_type;
		}
	}
	return -1;
}

int glDriver::getTextureComponentsByName(char* name) {
	for (int i = 0; i < TEXTURE_MODE; i++) {
		if (_strcmpi(name, textureModes[i].name) == 0) {
			return textureModes[i].tex_components;
		}
	}
	return -1;
}

void glDriver::calcMousePos(float x, float y) {
	Viewport vp = this->GetCurrentViewport();
	if ((x >= vp.x) && (x <= static_cast<float>(vp.width + vp.x)) &&	// Validate we are inside the valid zone of X
		(y >= vp.y) && (y <= static_cast<float>((vp.height + vp.y)))) {	// Validate we are inside the valid zone of Y

		float x_coord = (x - static_cast<float>(vp.x)) / static_cast<float>(vp.width);
		float y_coord = (y - static_cast<float>(vp.y)) / static_cast<float>(vp.height);
		x_coord -= 0.5f;	// Change scale from -0.5 to 0.5
		y_coord -= 0.5f;
		y_coord *= -1.0f;
		this->m_mouseX = x_coord;
		this->m_mouseY = y_coord;
	}
}
