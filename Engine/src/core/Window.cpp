// Window.cpp
// Spontz Demogroup

#include "Main.h"
#include "core/Window.h"

namespace Phoenix
{
	Window::Window()
		:
		m_demo(nullptr),
		m_timeCurrentFrame(0.0f),
		m_timeDelta(0.0f),
		m_timeLastFrame(0.0f),
		m_GLFWindow(nullptr),
		//m_imGui(nullptr),
		m_currentViewport{ 0,0,0,0 },
		m_currentViewportExprTK{ ViewportExprTK {0,0,0} }
	{
		PX_PROFILE_FUNCTION();

		// Fbo's config
		for (FboConfig& fboCfg : fboConfig) {
			fboCfg.format = "none";
			fboCfg.width = 0;
			fboCfg.height = 0;
			fboCfg.ratio = 0;
			fboCfg.numColorAttachments = 0;
		}

	}

	Window::~Window()
	{
		PX_PROFILE_FUNCTION();

		Shutdown();
	}

	void Window::OnUpdate()
	{
		PX_PROFILE_FUNCTION();

		glfwPollEvents();
		m_GLContext->SwapBuffers();
	}

	void Window::InitRender(bool clear)
	{
		InitOpenGLRenderStates();

		// Set the internal timer
		m_timeLastFrame = m_timeCurrentFrame;
		m_timeCurrentFrame = static_cast<float>(glfwGetTime());
		m_timeDelta = m_timeCurrentFrame - m_timeLastFrame;

		// set the viewport to the standard size
		SetCurrentViewport(GetFramebufferViewport());

		// clear some buffers if needed
		if (clear) {
			glClearColor(0, 0, 0, 0);

			if (m_Data.WindowProperties.Stencil) {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}
			else {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
		}
	}

	bool Window::checkError_(const char* file, int line)
	{
		if (m_demo->m_debug)
		{
			GLenum err = glGetError();
			std::string glError;

			if (err == GL_NO_ERROR)
				return false;
			switch (err) {
			case GL_INVALID_ENUM:
				glError = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				glError = "INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				glError = "INVALID_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				glError = "STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				glError = "STACK_UNDERLFOW";
				break;
			case GL_OUT_OF_MEMORY:
				glError = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				glError = "INVALID_FRAMEBUFFER_OPERATION";
				break;
			default:
				glError = "UNHANDLED ERROR";
				break;
			}
			Logger::error("GL Error: {0}, at {1}:{2}", glError, file, line);
			return true;
		}
		return false;
	}

	// Calculate mouse position in coordinates from 0 to 1
	glm::vec2 Window::CalcMousePos(glm::vec2 pos)
	{
		Viewport vp = GetCurrentViewport();
		glm::vec2 final_pos(0,0);

		if ((pos.x >= vp.x) && (pos.x <= static_cast<float>(vp.width + vp.x)) &&	// Validate we are inside the valid zone of X
			(pos.y >= vp.y) && (pos.y <= static_cast<float>((vp.height + vp.y)))) {	// Validate we are inside the valid zone of Y

			final_pos.x = (pos.x - static_cast<float>(vp.x)) / static_cast<float>(vp.width);
			final_pos.y = (pos.y - static_cast<float>(vp.y)) / static_cast<float>(vp.height);
			final_pos.x -= 0.5f;	// Change scale from -0.5 to 0.5
			final_pos.y -= 0.5f;
			final_pos.y *= -1.0f;
		}
		return final_pos;
	}

	void Window::SetWindowPos(int x, int y)
	{
		if (m_GLFWindow)
			glfwSetWindowPos(m_GLFWindow, x, y);
	}

	void Window::SetWindowSize(uint32_t width, uint32_t height)
	{
		if (m_GLFWindow)
			glfwSetWindowSize(m_GLFWindow, width, height); // TODO: Validar si esto genera un Callback (debería)
	}

	void Window::SetVSync(bool enabled)
	{
		PX_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.WindowProperties.VSync = enabled;
	}

	bool Window::IsVSync()
	{
		return m_Data.WindowProperties.VSync;
	}

	void Window::InitFbos()
	{
		////////////// efxBloom FBO Manager: internal FBO's that are being used by the engine effects
		{
			// Clear Fbo's, if there is any
			if (m_demo->m_efxBloomFbo.fbo.size() > 0) {
				Logger::info(LogLevel::low,	"Regenerating Bloom efx FBO's!...");
				m_demo->m_efxBloomFbo.clearFbos();
			}

			// init fbo's for Bloom
			FboConfig bloomFbo;
			bloomFbo.format = "RGB_16F";
			bloomFbo.numColorAttachments = 1;
			bloomFbo.ratio = 4;		// We use a division by 4 vs the framebuffer size
			bloomFbo.width = static_cast<float>(m_Data.WindowProperties.Width) / static_cast<float>(bloomFbo.ratio);
			bloomFbo.height = static_cast<float>(m_Data.WindowProperties.Height) / static_cast<float>(bloomFbo.ratio);

			int res = 0;
			for (int i = 0; i < EFXBLOOM_FBO_BUFFERS; i++) {
				if (m_demo->m_efxBloomFbo.addFbo(bloomFbo) >= 0) {
					Logger::info(LogLevel::low,	"EfxBloom Fbo {} uploaded: width: {:.0f}, height: {:.0f}, format: {}", i, bloomFbo.width, bloomFbo.height, bloomFbo.format);
				}
				else {
					Logger::error("Error in efxBloom Fbo definition: m_efxBloomFbo number {}", i);
				}
			}

		}

		////////////// efxAccum FBO Manager: internal FBO's that are being used by the engine effects
		// TODO: Que pasa si varios efectos de Accum se lanzan a la vez? no pueden usar la misma textura, asi que se mezclarán! deberíamos tener una fboConfig por cada efecto? es un LOCURON!!
		{
			// Clear Fbo's, if there is any
			if (m_demo->m_efxAccumFbo.fbo.size() > 0) {
				Logger::info(LogLevel::low, "Regenerating Accum efx FBO's!...");
				m_demo->m_efxAccumFbo.clearFbos();
			}

			// init fbo's for Accum
			FboConfig accumFbo;
			accumFbo.format = "RGBA_16F";
			accumFbo.numColorAttachments = 1;
			accumFbo.ratio = 1;	// Same size as the framebuffer
			accumFbo.width = static_cast<float>(m_Data.WindowProperties.Width) / static_cast<float>(accumFbo.ratio);
			accumFbo.height = static_cast<float>(m_Data.WindowProperties.Height) / static_cast<float>(accumFbo.ratio);

			int res = 0;
			for (int i = 0; i < EFXACCUM_FBO_BUFFERS; i++) {
				if (m_demo->m_efxAccumFbo.addFbo(accumFbo) >= 0) {
					Logger::info(LogLevel::low, "EfxAccum Fbo {} uploaded: width: {:.0f}, height: {:.0f}, format: {}", i, accumFbo.width, accumFbo.height, accumFbo.format);
				}
				else {
					Logger::error("Error in efxAccum Fbo definition: m_efxAccumFbo number {}", i);
				}
			}

		}

		////////////// FBO Manager: Generic FBO's that can be used by the user
		// Clear Fbo's, if there is any
		if (m_demo->m_fboManager.fbo.size() > 0) {
			Logger::info(LogLevel::low, "Ooops! we need to regenerate the FBO's! clearing generic FBO's first!");
			m_demo->m_fboManager.clearFbos();
		}

		// init fbo's
		for (int i = 0; i < FBO_BUFFERS; i++) {
			if (fboConfig[i].ratio != 0) {
				fboConfig[i].width = static_cast<float>(m_Data.WindowProperties.Width) / static_cast<float>(fboConfig[i].ratio);
				fboConfig[i].height = static_cast<float>(m_Data.WindowProperties.Height) / static_cast<float>(fboConfig[i].ratio);
			}

			if (m_demo->m_fboManager.addFbo(fboConfig[i]) >= 0) {
				Logger::info(LogLevel::low,	"Fbo {} uploaded: width: {:.0f}, height: {:.0f}, format: {}", i, fboConfig[i].width, fboConfig[i].height, fboConfig[i].format);
			}
			else {
				Logger::error("Error in FBO definition: FBO number {} has a non recongised format: '{}', please check 'graphics.spo' file.", i, fboConfig[i].format);
			}
		}
	}

	Viewport Window::GetFramebufferViewport() const
	{
		return Viewport::FromRenderTargetAndAspectRatio(m_Data.WindowProperties.Width, m_Data.WindowProperties.Height, m_Data.WindowProperties.AspectRatio);
	}

	void Window::SetFramebuffer()
	{
		m_demo->m_fboManager.unbind(false, false);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Restore the driver viewport
		//SetCurrentViewport(GetFramebufferViewport());
	}

	Viewport const& Window::GetCurrentViewport() const
	{
		return m_currentViewport;
	}

	void Window::SetCurrentViewport(Viewport const& viewport)
	{
		glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

		m_currentViewport = viewport;

		m_currentViewportExprTK.Width = static_cast<float>(viewport.width);
		m_currentViewportExprTK.Height = static_cast<float>(viewport.height);
		m_currentViewportExprTK.AspectRatio = m_currentViewport.GetAspectRatio();
	}

	bool Window::Init(std::string const &title)
	{
		PX_PROFILE_FUNCTION();
		
		// Set window Title
		m_Data.WindowProperties.Title = title;

		Logger::info(LogLevel::low, "Creating window {} ({}, {})", m_Data.WindowProperties.Title, m_Data.WindowProperties.Width, m_Data.WindowProperties.Height);

		{
			PX_PROFILE_SCOPE("glfwInit");
			if (!glfwInit()) {
				Logger::error("GLFW could not be initialized!");
				return false;
			}
		}
		
	
		{
			PX_PROFILE_SCOPE("glfwCreateWindow");
			if (m_Data.WindowProperties.DebugMode)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // TODO: Needed?
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // TODO: Needed?

			m_GLFWindow = glfwCreateWindow(
				(int)m_Data.WindowProperties.Width,
				(int)m_Data.WindowProperties.Height,
				m_Data.WindowProperties.Title.c_str(),
				m_Data.WindowProperties.Fullscreen ? glfwGetPrimaryMonitor() : nullptr,
				nullptr);
		}

		// Check if we need to overwride parameters
		if (m_demo->m_overrideWindowConfigParams) {
			m_Data.WindowProperties.Width = m_demo->m_windowWidth;
			m_Data.WindowProperties.Height = m_demo->m_windowHeight;
			glfwSetWindowSize(m_GLFWindow, m_demo->m_windowWidth, m_demo->m_windowHeight);
			SetWindowPos(m_demo->m_windowPosX, m_demo->m_windowPosY);
		}

		m_GLContext = std::make_unique<GLContext>(m_GLFWindow);
		m_GLContext->Init();

		glfwSetWindowUserPointer(m_GLFWindow, &m_Data);

		// Enable multisampling (aka anti-aliasing)
		if (m_Data.WindowProperties.Multisampling)
		{
			glfwWindowHint(GLFW_SAMPLES, 4); // This does mean that the size of all the buffers is increased by 4
			glEnable(GL_MULTISAMPLE);
		}

		// Set VSync
		SetVSync(m_Data.WindowProperties.VSync);

		// Set GLFW callbacks: TODO: We can enable or disable events by just commenting this Callbacks
		glfwSetErrorCallback(glfwErrorCallback);
		glfwSetWindowSizeCallback(m_GLFWindow, glfwWindowSizeCallback);
		glfwSetWindowCloseCallback(m_GLFWindow, glfwWindowCloseCallback);
		glfwSetKeyCallback(m_GLFWindow, glfwKeyCallback);
		glfwSetCharCallback(m_GLFWindow, glfwCharCallback);
		glfwSetMouseButtonCallback(m_GLFWindow, glfwMouseButtonCallback);
		glfwSetScrollCallback(m_GLFWindow, glfwMouseScrollCallback);
		glfwSetCursorPosCallback(m_GLFWindow, glMouseMoveCallback);
		
				
		// Enable debug mode and debug callback
		if (m_Data.WindowProperties.DebugMode) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glfwDebugMessageCallback, 0);
			// If you want to disable all error messages, except the API error messages, then you have to disable all messages first and the enable explicitly the API error messages:
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
			glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
		}

		// init fbo's
		InitFbos();

		// Init internal timer
		m_timeCurrentFrame = static_cast<float>(glfwGetTime());

		return true;
	}

	void Window::InitOpenGLRenderStates()
	{
		glDisable(GL_BLEND);						// blending disabled
		glBlendFunc(GL_ONE, GL_ONE);				// Additive blending function by default
		glBlendEquation(GL_FUNC_ADD);				// ADD function by default

		glDisable(GL_CULL_FACE);					// cull face disabled
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// draw cwise and ccwise in fill mode

		glEnable(GL_DEPTH_TEST);					// depth test enabled
		glDepthFunc(GL_LEQUAL);						// depth test comparison function set to LEQUAL

		// Init lights colors, fbo's, shader ID's and texture States
		DEMO->m_lightManager.initAllLightsColors();
		DEMO->m_fboManager.unbind(true, true);
		DEMO->m_shaderManager.unbindShaders();
		DEMO->m_textureManager.initTextureStates();
	}

	void Window::Shutdown()
	{
		PX_PROFILE_FUNCTION();

		if (m_GLFWindow)
		{
			glfwDestroyWindow(m_GLFWindow);
			glfwTerminate();
		}
	}

	void Window::glfwDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		Logger::info(
			LogLevel::low,
			"Error GL callback: {} type = 0x{:x}, severity = 0x{:x}, message = {}",
			type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "",
			type,
			severity,
			message
		);
	}

	void Window::glfwErrorCallback(int, const char* err_str)
	{
		Logger::error("GLFW Error: {}", err_str);
	}

	void Window::glfwWindowSizeCallback(GLFWwindow* p_glfw_window, int width, int height) // TODO: Impementar el resizing
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);
		data.WindowProperties.Width = width;
		data.WindowProperties.Height = height;

		WindowResizeEvent event(width, height);
		data.EventCallback(event);
	}


	void Window::glfwWindowCloseCallback(GLFWwindow* p_glfw_window)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);
		WindowCloseEvent event;
		data.EventCallback(event);
	}

	bool Window::OnMouseButtonPressed(uint16_t button)
	{
		switch (button) {
		case Mouse::BUTTON_RIGHT:
			m_MouseStatus.RightClick = true;
			glm::vec2 MousePos = CalcMousePos(glm::vec2(m_MouseStatus.PosX, m_MouseStatus.PosY));
			Logger::sendEditor("Mouse position [{:.4f}, {:.4f}]", MousePos.x, MousePos.y);
			break;
		case Mouse::BUTTON_LEFT:
			m_MouseStatus.LeftClick = true;
			m_MouseStatus.LastPosX = m_MouseStatus.PosX;
			m_MouseStatus.LastPosY = m_MouseStatus.PosY;
			break;
		case Mouse::BUTTON_MIDDLE:
			m_MouseStatus.MiddleClick = true;
			break;
		}
		return true;
	}

	bool Window::OnMouseButtonReleased(uint16_t button)
	{
		switch (button) {
		case Mouse::BUTTON_RIGHT:
			m_MouseStatus.RightClick = false;
			break;
		case Mouse::BUTTON_LEFT:
			m_MouseStatus.LeftClick = false;
			m_MouseStatus.LastPosX = m_MouseStatus.PosX;
			m_MouseStatus.LastPosY = m_MouseStatus.PosY;
			break;
		case Mouse::BUTTON_MIDDLE:
			m_MouseStatus.MiddleClick = false;
			break;
		}
		return true;
	}

	bool Window::OnMouseMoved(float PosX, float PosY)
	{
		m_MouseStatus.PosX = PosX;
		m_MouseStatus.PosY = PosY;
		if (m_MouseStatus.LeftClick || m_MouseStatus.RightClick) {
			float x = m_MouseStatus.PosX;
			float y = m_MouseStatus.PosY;
			// Move camera with Left click
			if (m_MouseStatus.LeftClick) {

				float xoffset = x - m_MouseStatus.LastPosX;
				float yoffset = m_MouseStatus.LastPosY - y; // reversed since y-coordinates go from bottom to top

				m_MouseStatus.LastPosX = x;
				m_MouseStatus.LastPosY = y;

				m_demo->m_pActiveCamera->processMouseMovement(xoffset, yoffset);
			}
			// Capture mouse position with Right click
			if (m_MouseStatus.RightClick) {
				//glm::vec2 MousePos = CalcMousePos(glm::vec2(m_MouseStatus.PosX, m_MouseStatus.PosY));
				//Logger::sendEditor("Mouse position [{:.4f}, {:.4f}]", MousePos.x, MousePos.y);
			}
		}
		return true;
	}

	bool Window::OnMouseScrolled(float OffsetX, float OffsetY)
	{
		m_demo->m_pActiveCamera->processMouseScroll(OffsetY);
		return true;
	}
	
	void Window::OnWindowResize(uint32_t width, uint32_t height)
	{
		m_Data.WindowProperties.Width = width;
		m_Data.WindowProperties.Height = height;

		m_MouseStatus.LastPosX = static_cast<float>(width) / 2.0f;
		m_MouseStatus.LastPosY = static_cast<float>(height) / 2.0f;

		// Recalculate viewport sizes
		SetCurrentViewport(GetFramebufferViewport());

		// Recalculate fbo's with the new window size
		InitFbos();

		InitRender(true);
		//Logger::info(LogLevel::low, "Window Size: %d,%d", width, height);
		//Logger::info(LogLevel::low, "Current viewport Size: %d,%d, Pos: %d, %d", m_current_viewport.width, m_current_viewport.height, m_current_viewport.x, m_current_viewport.y);
		//Logger::info(LogLevel::low, "Current viewport exprtk: %.2f,%.2f, aspect: %.2f", m_exprtkCurrentViewport.width, m_exprtkCurrentViewport.height, m_exprtkCurrentViewport.aspect_ratio);
	}

	void Window::OnProcessInput()
	{
		PX_PROFILE_FUNCTION();

		if (m_demo->m_debug) {
			if (Input::IsKeyPressed(Key::CAM_FORWARD))
				m_demo->m_pActiveCamera->processKeyboard(CameraMovement::FORWARD, m_timeDelta);
			if (Input::IsKeyPressed(Key::CAM_BACKWARD))
				m_demo->m_pActiveCamera->processKeyboard(CameraMovement::BACKWARD, m_timeDelta);
			if (Input::IsKeyPressed(Key::CAM_STRAFE_LEFT))
				m_demo->m_pActiveCamera->processKeyboard(CameraMovement::LEFT, m_timeDelta);
			if (Input::IsKeyPressed(Key::CAM_STRAFE_RIGHT))
				m_demo->m_pActiveCamera->processKeyboard(CameraMovement::RIGHT, m_timeDelta);
			if (Input::IsKeyPressed(Key::CAM_ROLL_RIGHT))
				m_demo->m_pActiveCamera->processKeyboard(CameraMovement::ROLL_RIGHT, m_timeDelta);
			if (Input::IsKeyPressed(Key::CAM_ROLL_LEFT))
				m_demo->m_pActiveCamera->processKeyboard(CameraMovement::ROLL_LEFT, m_timeDelta);
		}
	}


	void Window::glfwKeyCallback(GLFWwindow* p_glfw_window, int key, int scancode, int action, int mods)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);
		
		switch (action)
		{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.EventCallback(event);
				break;
			}
		}
	}

	void Window::glfwCharCallback(GLFWwindow* p_glfw_window, unsigned int keycode)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);

		KeyTypedEvent event(keycode);
		data.EventCallback(event);
	}

	void Window::glfwMouseButtonCallback(GLFWwindow* p_glfw_window, int button, int action, int mods)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);

		switch (action)
		{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
		}
	}

	void Window::glfwMouseScrollCallback(GLFWwindow* p_glfw_window, double xOffset, double yOffset)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);

		MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
		data.EventCallback(event);
	}

	void Window::glMouseMoveCallback(GLFWwindow* p_glfw_window, double xPos, double yPos)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);

		MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
		data.EventCallback(event);
	}

}