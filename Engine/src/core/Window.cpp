// Window.cpp
// Spontz Demogroup

#include "Main.h"
#include "core/Window.h"

namespace Phoenix
{
	Window::Window(const WindowProps& props)
	{
		PX_PROFILE_FUNCTION();

		Init(props);
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

	void Window::SetWindowPos(int x, int y)
	{
		if (m_GLFWindow)
			glfwSetWindowPos(m_GLFWindow, x, y);
	}

	void Window::SetWindowSize(uint32_t width, uint32_t height)
	{
		if (m_GLFWindow)
			glfwSetWindowSize(m_GLFWindow, width, height);
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

	void Window::Init(const WindowProps& props)
	{
		PX_PROFILE_FUNCTION();

		m_Data.WindowProperties = props;

		Logger::info(LogLevel::low, "Creating window {} ({}, {})", m_Data.WindowProperties.Title, m_Data.WindowProperties.Width, m_Data.WindowProperties.Height);

		{
			PX_PROFILE_SCOPE("glfwInit");
			if (!glfwInit()) {
				Logger::error("GLFW could not be initialized!");
				return;
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

		// Set GLFW callbacks
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

	void Window::glfwWindowSizeCallback(GLFWwindow* p_glfw_window, int width, int height)
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
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, 1);
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

		MouseScrolledEvent event((float)xOffset, (float)yOffset);
		data.EventCallback(event);
	}

	void Window::glMouseMoveCallback(GLFWwindow* p_glfw_window, double xPos, double yPos)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_glfw_window);

		MouseMovedEvent event((float)xPos, (float)yPos);
		data.EventCallback(event);
	}

	


}