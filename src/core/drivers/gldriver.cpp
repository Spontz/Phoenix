// gldriver.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/events.h"

// Demo states to show in the drawTiming information ****************

char* stateStr[] = {
	"play",
	"play - RW",
	"play - FF",
	"paused",
	"paused - RW",
	"paused - FF"
};

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

void error_callback(int, const char* err_str)
{
	LOG->Error("GLFW Error: %s", err_str);
}

void glDriver::GLFWWindowSizeCallback(GLFWwindow* p_glfw_window, int width, int height) {
	GLDRV->OnWindowSizeChanged(p_glfw_window, width, height);
}

void glDriver::OnWindowSizeChanged(GLFWwindow* p_glfw_window, int width, int height) {
	// TODO/HACK: Add min size to window @ OS api level and get rid of this
	width = std::max(width, 1);
	height = std::max(height, 1);

	script__gl_width__framebuffer_width_ = width;
	script__gl_height__framebuffer_height_ = height;

	// RT will be set to FB later
	//GLDRV->current_rt_width_ = width;
	//GLDRV->current_rt_height_ = height;

	mouse_lastxpos = static_cast<float>(width) / 2.0f;
	mouse_lastypos = static_cast<float>(height) / 2.0f;

	// Recalculate viewport sizes
	SetCurrentViewport(GetFramebufferViewport());

	// Recalculate fbo's with the new window size
	initFbos();

	initRender(true);
}

void mouse_callback(GLFWwindow* p_glfw_window, double xpos, double ypos)
{
	if (DEMO->debug) {
		float x = static_cast<float>(xpos);
		float y = static_cast<float>(ypos);

		// Move camera with Left click
		if (glfwGetMouseButton(p_glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			float xoffset = x - GLDRV->mouse_lastxpos;
			float yoffset = GLDRV->mouse_lastypos - y; // reversed since y-coordinates go from bottom to top

			GLDRV->mouse_lastxpos = x;
			GLDRV->mouse_lastypos = y;

			DEMO->camera->ProcessMouseMovement(xoffset, yoffset);
		}
		if (glfwGetMouseButton(p_glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			GLDRV->mouse_lastxpos = x;
			GLDRV->mouse_lastypos = y;
		}
		// Capture mouse position with Right click
		if (glfwGetMouseButton(p_glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			GLDRV->calcMousePos(x, y);
		}
		

	}
}

void mouseButton_callback(GLFWwindow* p_glfw_window, int button, int action, int mods)
{
	if (DEMO->debug) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			GLDRV->calcMousePos(GLDRV->mouse_lastxpos, GLDRV->mouse_lastypos);
			LOG->SendEditor("Mouse pos [%.4f, %.4f]", GLDRV->mouse_x, GLDRV->mouse_y);
		}
				}
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is
// called
void scroll_callback(GLFWwindow* p_glfw_window, double xoffset, double yoffset)
{
	DEMO->camera->ProcessMouseScroll((float)yoffset);
}

void APIENTRY glErrorCallback(GLenum source, GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	LOG->Info(LOG_LOW, "Error GL callback: %s type = 0x%x, severity = 0x%x, message = %s",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void key_callback(GLFWwindow* p_glfw_window, int key, int scancode, int action, int mods) {

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
			else if (key == KEY_SHOWSCENEINFO)
				DEMO->drawSceneInfo = !DEMO->drawSceneInfo;
			else if (key == KEY_SHOWFBO) {
				DEMO->drawFbo++;
				if (DEMO->drawFbo >= 7)
					DEMO->drawFbo = 0;
			}
			else if (key == KEY_CHANGEATTACH) {
				DEMO->drawFboAttachment++;
				if (DEMO->drawFboAttachment >= GLDRV_MAX_COLOR_ATTACHMENTS)
					DEMO->drawFboAttachment = 0;
			}

			else if (key == KEY_CAPTURE) {
				DEMO->camera->CapturePos();
			}
			else if (key == KEY_CAMRESET) {
				DEMO->camera->Reset();
			}
			else if (key == KEY_MULTIPLIER) {
				DEMO->camera->MovementSpeed *= 2.0f;
			}
			else if (key == KEY_DIVIDER) {
				DEMO->camera->MovementSpeed /= 2.0f;
				if (DEMO->camera->MovementSpeed < 1.0f)
					DEMO->camera->MovementSpeed = 1.0f;
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

// glDriver *********************************************************

glDriver& glDriver::GetInstance() {
	static glDriver obj;
	return obj;
}

void glDriver::ProcessInput()
{
	if (DEMO->debug) {
		if (glfwGetKey(p_glfw_window_, KEY_FORWARD) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CameraMovement::FORWARD, GLDRV->TimeDelta);
		if (glfwGetKey(p_glfw_window_, KEY_BACKWARD) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CameraMovement::BACKWARD, GLDRV->TimeDelta);
		if (glfwGetKey(p_glfw_window_, KEY_STRAFELEFT) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CameraMovement::LEFT, GLDRV->TimeDelta);
		if (glfwGetKey(p_glfw_window_, KEY_STRAFERIGHT) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CameraMovement::RIGHT, GLDRV->TimeDelta);
		if (glfwGetKey(p_glfw_window_, KEY_ROLLRIGHT) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CameraMovement::ROLL_RIGHT, GLDRV->TimeDelta);
		if (glfwGetKey(p_glfw_window_, KEY_ROLLLEFT) == GLFW_PRESS)
			DEMO->camera->ProcessKeyboard(CameraMovement::ROLL_LEFT, GLDRV->TimeDelta);
	}

}

glDriver::glDriver()
	:
	current_rt_height_(0),
	current_rt_width_(0),
	TimeCurrentFrame(0.0f),
	TimeDelta(0.0f),
	TimeLastFrame(0.0f),
	p_glfw_window_(nullptr),

	script__gl_width__framebuffer_width_(640),
	script__gl_height__framebuffer_height_(480),
	script__gl_aspect__framebuffer_viewport_aspect_ratio_(UIntFraction{ 16,9 }.GetRatio()),

	exprtk__aspectRatio__current_viewport_aspect_ratio_(0.0f),
	exprtk__vpHeight__current_viewport_height_(0.0f),
	exprtk__vpWidth__current_viewport_width_(0.0f),

	current_viewport_{ 0,0,0,0 },

	mouse_lastxpos(0),
	mouse_lastypos(0),
	mouse_x(0),
	mouse_y(0),
	fullScreen(0),
	stencil(0),
	accum(0),
	multisampling(0),
	vsync(0),
	gamma(1.0f),
	fbo{ tGLFboFormat{0.0f,0.0f,0,0,0,0,0,nullptr,0} }
{
	// hack:
	for (auto i = 0; i < FBO_BUFFERS; ++i) {
		fbo[i].width = 0;
		fbo[i].height = 0;
		fbo[i].ratio = 0;
	}
}

void glDriver::initFramework() {
	// Initialize the library
	if (!glfwInit()) {
		LOG->Error("GL Framework could not be initialized!");
		return;
	}
	else {
		LOG->Info(LOG_MED, "GLFW library version is: %s", glfwGetVersionString());
		glfwSetErrorCallback(error_callback);
	}
}

void glDriver::initGraphics() {
	mouse_lastxpos = script__gl_width__framebuffer_width_ / 2.0f;
	mouse_lastypos = script__gl_height__framebuffer_height_ / 2.0f;

	// Create a windowed mode window and its OpenGL context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	p_glfw_window_ = glfwCreateWindow(
		script__gl_width__framebuffer_width_,
		script__gl_height__framebuffer_height_,
		DEMO->demoName,
		GLDRV->fullScreen ? glfwGetPrimaryMonitor() : nullptr,
		nullptr
	);

	if (!p_glfw_window_)
		glfwTerminate();

	// Enable multisampling (aka anti-aliasing)
	if (this->multisampling)
		glfwWindowHint(GLFW_SAMPLES, 4); // This does mean that the size of all the buffers is increased by 4

	// Make the window's context current
	glfwMakeContextCurrent(p_glfw_window_);

	// Configure GLFW callbacks
	glfwSetWindowSizeCallback(p_glfw_window_, GLFWWindowSizeCallback);
	glfwSetKeyCallback(p_glfw_window_, key_callback);
	glfwSetCursorPosCallback(p_glfw_window_, mouse_callback);
	glfwSetMouseButtonCallback(p_glfw_window_, mouseButton_callback);

	glfwSetScrollCallback(p_glfw_window_, scroll_callback);

	// Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		LOG->Error("Failed to initialize GLAD");

	// Enable multisampling state (aka anti-aliasing)
	if (this->multisampling)
		glEnable(GL_MULTISAMPLE);

	// Calculate the Viewport sizes
	SetCurrentViewport(GetFramebufferViewport());

	// Init render
	initRender(true);

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
	initFbos();

	// Init internal timer
	TimeCurrentFrame = static_cast<float>(glfwGetTime());
}

void glDriver::initStates() {
	glDisable(GL_BLEND);						// blending disabled
	glBlendFunc(GL_ONE, GL_ONE);				// Additive blending function by default
	glBlendEquation(GL_FUNC_ADD);				// ADD function by default

	glDisable(GL_CULL_FACE);					// cull face disabled
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// draw cwise and ccwise in fill mode

	glEnable(GL_DEPTH_TEST);					// depth test enabled
	glDepthFunc(GL_LEQUAL);						// depth test comparison function set to LEQUAL - TODO: Should be LESS according learnopengl.com

	// Init lights colors, fbo's, shader ID's and texture States
	DEMO->lightManager.initAllLightsColors();
	DEMO->fboManager.unbind(true, true);
	DEMO->shaderManager.unbindShaders();
	DEMO->textureManager.initTextureStates();
}

void glDriver::initRender(int clear) {
	// Vsync Management
	glfwSwapInterval(vsync); // 0 -Disabled, 1-60pfs, 2-30fps, 3-20fps,...

	// reset the default gl state
	this->initStates();

	// Set the internal timer
	TimeLastFrame = TimeCurrentFrame;
	TimeCurrentFrame = static_cast<float>(glfwGetTime());
	TimeDelta = TimeCurrentFrame - TimeLastFrame;

	// set the viewport to the standard size
	// setViewport(vpXOffset, vpYOffset, static_cast<int>(vpWidth), static_cast<int>(vpHeight));
	SetCurrentViewport(GetFramebufferViewport());

	// clear some buffers if needed
	if (clear) {
		glClearColor(0, 0, 0, 0);

		if (this->stencil > 0) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
		else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}
}


/*
Moved to Viewport::FromRenderTargetAndAspectRatio

// Calculate the viewport size according the given Width, Height and Aspect Ratio
this->vpWidth = (float)this->width;
this->vpHeight = (float)this->height;
this->vpXOffset = 0; // Width offset (X)
this->vpYOffset = 0; // Height offset (Y)

// Si la anchura que necesitamos es demasiado grande, hya q recalcular una nueva anchura y poner unos bordes a los lados
if (((float)this->width / this->AspectRatio) > (float)this->height) {
	vpWidth = (float)this->height * this->AspectRatio;
	this->vpXOffset = (int)(((float)this->width - this->vpWidth) / 2.0);
}

// Si la altura que necesitamos es demasiado grande, hya q recalcular una nueva altura y poner unos bordes arriba y abajo
if (((float)this->height * this->AspectRatio) > (float)this->width) {
	this->vpHeight = (float)this->width / this->AspectRatio;
	this->vpYOffset = (int)(((float)this->height - this->vpHeight) / 2.0);
}

// Prevent zero division
if (vpWidth < 1.0)
	vpWidth = 1.0;
if (vpHeight < 1.0)
	vpHeight = 1.0;

LOG->Info(LOG_LOW, "Requested resolution (W,H): %d,%d. Aspect ratio: %.4f", this->width, this->height, this->AspectRatio);
LOG->Info(LOG_LOW, "The viewport will be placed at pos (X,Y): %d,%d with size (W,H): %d,%d", this->vpXOffset, this->vpYOffset, (int)this->vpWidth, (int)this->vpHeight);
*/


Viewport glDriver::GetFramebufferViewport() const {
	return Viewport::FromRenderTargetAndAspectRatio(
		script__gl_width__framebuffer_width_,
		script__gl_height__framebuffer_height_,
		script__gl_aspect__framebuffer_viewport_aspect_ratio_
	);
}

Viewport const& glDriver::GetCurrentViewport() const {
	return current_viewport_;
}

float glDriver::GetFramebufferAspectRatio() const {
	return static_cast<float>(script__gl_width__framebuffer_width_) / static_cast<float>(script__gl_height__framebuffer_height_);
}

void glDriver::SetCurrentViewport(Viewport const& viewport) {
	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

	current_viewport_ = viewport;

	exprtk__vpWidth__current_viewport_width_ = static_cast<float>(viewport.width);
	exprtk__vpHeight__current_viewport_height_ = static_cast<float>(viewport.height);
	exprtk__aspectRatio__current_viewport_aspect_ratio_ = current_viewport_.GetAspectRatio();
}

void glDriver::SetFramebuffer() {
	DEMO->fboManager.unbind(false, false);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Restore the driver viewport
	//SetCurrentViewport(GetFramebufferViewport());
}

void glDriver::initFbos() {
	////////////// efxBloom FBO Manager: internal FBO's that are being used by the engine effects
	{
		// Clear Fbo's, if there is any
		if (DEMO->efxBloomFbo.fbo.size() > 0) {
			LOG->Info(LOG_LOW, "Ooops! we need to regenerate the Bloom efx FBO's! clearing efx FBO's first!");
			DEMO->efxBloomFbo.clearFbos();
		}

		// init fbo's for Bloom
		tGLFboFormat bloomFbo;
		bloomFbo.format = "RGB_16F";
		bloomFbo.numColorAttachments = 1;
		bloomFbo.ratio = 4;
		bloomFbo.width = static_cast<float>(script__gl_width__framebuffer_width_) / static_cast <float>(bloomFbo.ratio);
		bloomFbo.height = static_cast<float>(script__gl_height__framebuffer_height_) / static_cast <float>(bloomFbo.ratio);
		bloomFbo.tex_iformat = getTextureInternalFormatByName(bloomFbo.format);
		bloomFbo.tex_format = getTextureFormatByName(bloomFbo.format);
		bloomFbo.tex_type = getTextureTypeByName(bloomFbo.format);
		bloomFbo.tex_components = getTextureComponentsByName(bloomFbo.format);

		int res = 0;
		for (int i = 0; i < EFXBLOOM_FBO_BUFFERS; i++) {
			res = DEMO->efxBloomFbo.addFbo(bloomFbo.format, (int)bloomFbo.width, (int)bloomFbo.height, bloomFbo.tex_iformat, bloomFbo.tex_format, bloomFbo.tex_type, bloomFbo.tex_components, bloomFbo.numColorAttachments);
			if (res >= 0)
				LOG->Info(LOG_LOW, "EfxBloom Fbo %i uploaded: width: %.0f, height: %.0f, format: %s, components: %i, GLformat: %i, GLiformat: %i, GLtype: %i", i, bloomFbo.width, bloomFbo.height, bloomFbo.format, bloomFbo.tex_components, bloomFbo.tex_format, bloomFbo.tex_iformat, bloomFbo.tex_type);
			else
				LOG->Error("Error in efxBloom Fbo definition: Efx_Fbo number %i has a non recongised format: '%s', please blame the coder.", i, bloomFbo.format);
		}

	}

	////////////// efxAccum FBO Manager: internal FBO's that are being used by the engine effects
	// TODO: Que pasa si varios efectos de Accum se lanzan a la vez? no pueden usar la misma textura, asi que se mezclarán! deberíamos tener una fbo por cada efecto? es un LOCURON!!
	{
		// Clear Fbo's, if there is any
		if (DEMO->efxAccumFbo.fbo.size() > 0) {
			LOG->Info(LOG_LOW, "Ooops! we need to regenerate the Accum efx FBO's! clearing efx FBO's first!");
			DEMO->efxAccumFbo.clearFbos();
		}

		// init fbo's for Accum
		tGLFboFormat accumFbo;
		accumFbo.format = "RGBA_16F";
		accumFbo.numColorAttachments = 1;
		accumFbo.ratio = 1;
		accumFbo.width = static_cast<float>(script__gl_width__framebuffer_width_) / static_cast <float>(accumFbo.ratio);
		accumFbo.height = static_cast<float>(script__gl_height__framebuffer_height_) / static_cast <float>(accumFbo.ratio);
		accumFbo.tex_iformat = getTextureInternalFormatByName(accumFbo.format);
		accumFbo.tex_format = getTextureFormatByName(accumFbo.format);
		accumFbo.tex_type = getTextureTypeByName(accumFbo.format);
		accumFbo.tex_components = getTextureComponentsByName(accumFbo.format);

		int res = 0;
		for (int i = 0; i < EFXACCUM_FBO_BUFFERS; i++) {
			res = DEMO->efxAccumFbo.addFbo(accumFbo.format, (int)accumFbo.width, (int)accumFbo.height, accumFbo.tex_iformat, accumFbo.tex_format, accumFbo.tex_type, accumFbo.tex_components, accumFbo.numColorAttachments);
			if (res >= 0)
				LOG->Info(LOG_LOW, "EfxAccum Fbo %i uploaded: width: %.0f, height: %.0f, format: %s, components: %i, GLformat: %i, GLiformat: %i, GLtype: %i", i, accumFbo.width, accumFbo.height, accumFbo.format, accumFbo.tex_components, accumFbo.tex_format, accumFbo.tex_iformat, accumFbo.tex_type);
			else
				LOG->Error("Error in efxAccum Fbo definition: Efx_Fbo number %i has a non recongised format: '%s', please blame the coder.", i, accumFbo.format);
		}

	}

	////////////// FBO Manager: Generic FBO's that can be used by the user
	// Clear Fbo's, if there is any
	if (DEMO->fboManager.fbo.size() > 0) {
		LOG->Info(LOG_LOW, "Ooops! we need to regenerate the FBO's! clearing generic FBO's first!");
		DEMO->fboManager.clearFbos();
	}

	// init fbo's
	for (int i = 0; i < FBO_BUFFERS; i++) {
		if (((this->fbo[i].width != 0) && (this->fbo[i].height != 0)) || (this->fbo[i].ratio != 0)) {
			if (this->fbo[i].ratio != 0) {
				this->fbo[i].width = (float)(this->script__gl_width__framebuffer_width_ / this->fbo[i].ratio);
				this->fbo[i].height = (float)(this->script__gl_height__framebuffer_height_ / this->fbo[i].ratio);
			}

			this->fbo[i].tex_iformat = getTextureInternalFormatByName(this->fbo[i].format);
			this->fbo[i].tex_format = getTextureFormatByName(this->fbo[i].format);
			this->fbo[i].tex_type = getTextureTypeByName(this->fbo[i].format);
			this->fbo[i].tex_components = getTextureComponentsByName(this->fbo[i].format);
			// Check if the format is valid
			if (this->fbo[i].tex_format > 0) {
				DEMO->fboManager.addFbo(this->fbo[i].format, (int)this->fbo[i].width, (int)this->fbo[i].height, this->fbo[i].tex_iformat, this->fbo[i].tex_format, this->fbo[i].tex_type, this->fbo[i].tex_components, this->fbo[i].numColorAttachments);
				LOG->Info(LOG_LOW, "Fbo %i uploaded: width: %.0f, height: %.0f, format: %s, components: %i, GLformat: %i, GLiformat: %i, GLtype: %i", i, this->fbo[i].width, this->fbo[i].height, this->fbo[i].format, this->fbo[i].tex_components, this->fbo[i].tex_format, this->fbo[i].tex_iformat, this->fbo[i].tex_type);
			}
			else {
				LOG->Error("Error in FBO definition: FBO number %i has a non recongised format: '%s', please check 'graphics.spo' file.", i, this->fbo[i].format);
			}
		}
	}

}

int glDriver::WindowShouldClose() {
	return glfwWindowShouldClose(p_glfw_window_);
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
	glfwSwapBuffers(p_glfw_window_);
}

void glDriver::close() {
	glfwSetWindowShouldClose(p_glfw_window_, GL_TRUE);
	glfwTerminate();
}

// Draws the information of all the scenes that are being drawn
void glDriver::drawSceneInfo()
{
	Section *ds;
	int sec_id;
	string message = "Drawing Stack:\n--------------------------------------------\n" ;

	for (int i = 0; i < DEMO->sectionManager.execSection.size(); i++) {
		sec_id = DEMO->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = DEMO->sectionManager.section[sec_id];
		message += ds->debug();									// Loads the debug message from the section and adds to the queue
	}

	std::stringstream ss(message);
	std::string to;

	float pos = 0;
	while (std::getline(ss, to, '\n')) {
		DEMO->text->RenderText(to, 0.2f, 0.9f + pos, 0.15f, glm::vec3(1, 1, 1));
		pos -= 0.07f;
	}
}

void glDriver::drawFps() {
	DEMO->text->glPrintf(-1, 0.9f, "FPS: %.0f", DEMO->fps);
}

void glDriver::drawTiming() {
	char* state;
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
	DEMO->text->glPrintf(-1, 0.7f, "sound: %0.1f", BASSDRV->sound_cpu());
	DEMO->text->glPrintf(-1, 0.6f, "texmem: %.2fmb", float(DEMO->textureManager.mem + DEMO->fboManager.mem));
	DEMO->text->glPrintf(-1, 0.5f, "Cam Speed: %.0f", DEMO->camera->MovementSpeed);
	DEMO->text->glPrintf(-1, 0.4f, "Cam Pos: %.1f,%.1f,%.1f", DEMO->camera->Position.x, DEMO->camera->Position.y, DEMO->camera->Position.z);
	DEMO->text->glPrintf(-1, 0.3f, "Cam Front: %.1f,%.1f,%.1f", DEMO->camera->Front.x, DEMO->camera->Front.y, DEMO->camera->Front.z);
	DEMO->text->glPrintf(-1, 0.2f, "Cam Yaw: %.1f, Pitch: %.1f, Roll: %.1f, Zoom: %.1f", DEMO->camera->Yaw, DEMO->camera->Pitch, DEMO->camera->Roll, DEMO->camera->Zoom);
	DEMO->text->glPrintf(-1, 0.1f, state);
}

// Draw the Fbo output
void glDriver::drawFbo() {
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	int fbo_num_min = ((DEMO->drawFbo - 1) * NUM_FBO_DEBUG);
	int fbo_num_max = (NUM_FBO_DEBUG - 1) + ((DEMO->drawFbo - 1) * NUM_FBO_DEBUG);
	int fbo_attachment = DEMO->drawFboAttachment;
	DEMO->text->glPrintf(-1, -0.5f, "Showing FBO's: %d to %d - Attachment: %d", fbo_num_min, fbo_num_max, fbo_attachment);
	for (int i = 0; i < NUM_FBO_DEBUG; i++) {
		int fbo_num = (0 + i) + ((DEMO->drawFbo - 1) * NUM_FBO_DEBUG);
		RES->Draw_Obj_QuadFBO_Debug(i, fbo_num, fbo_attachment);
	}
	glEnable(GL_DEPTH_TEST);
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
		this->mouse_x = x_coord;
		this->mouse_y = y_coord;
	}
}
