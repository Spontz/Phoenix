// Window.h
// Window abstraction class
// Spontz Demogroup

#pragma once

#include <sstream>
#include <memory>

#include "Main.h"
#include "core/events/Event.h"

#include "core/drivers/GLContext.h"

#include <GLFW/glfw3.h>

namespace Phoenix {

	struct WindowProps
	{
		std::string	Title;
		uint32_t	Width;
		uint32_t	Height;
		float		AspectRatio;
		bool		DebugMode;
		bool		VSync;
		bool		Fullscreen;
		bool		Multisampling;
		bool		Stencil;

		WindowProps()
			:
			Title("Phoenix :: Spontz Demoengine"),
			Width(640),
			Height(480),
			AspectRatio(16.0f/9.0f),
			DebugMode(false),
			VSync(true),
			Fullscreen(false),
			Multisampling(false),
			Stencil(false)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(const WindowProps& props = WindowProps());
		~Window();

		void OnUpdate();

		uint32_t GetWidth() const { return m_Data.WindowProperties.Width; }
		uint32_t GetHeight() const { return m_Data.WindowProperties.Height; }

		// Window attributes
		void SetWindowPos(int x, int y);
		void SetWindowSize(uint32_t width, uint32_t height);
		void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; };
		void SetVSync(bool enabled);
		bool IsVSync();

		void* GetNativeWindow() const { return m_GLFWindow; };
	
		const std::string				getGLFWVersion() { return m_GLContext->getGLFWVersion(); };
		const std::string				getGLVersion() { return m_GLContext->getGLVersion(); };
		const std::string				getGLVendor() { return m_GLContext->getGLVendor(); };
		const std::string				getGLRenderer() { return m_GLContext->getGLRenderer(); };;
		const std::vector<std::string>	getGLExtensions() { return m_GLContext->getGLExtensions(); };;

	private:
		void Init(const WindowProps& props);
		void Shutdown();
		// Callbacks
		static void glfwDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
		static void glfwErrorCallback(int, const char* err_str);
		static void glfwKeyCallback(GLFWwindow* p_glfw_window, int key, int scancode, int action, int mods);
		static void glfwCharCallback(GLFWwindow* p_glfw_window, unsigned int keycode);
		static void glfwMouseScrollCallback(GLFWwindow* p_glfw_window, double xOffset, double yOffset);
		static void glMouseMoveCallback(GLFWwindow* p_glfw_window, double xPos, double yPos);
		static void glfwMouseButtonCallback(GLFWwindow* p_glfw_window, int button, int action, int mods);
		static void glfwWindowSizeCallback(GLFWwindow* p_glfw_window, int width, int height);
		static void glfwWindowCloseCallback(GLFWwindow* p_glfw_window);

	private:
		GLFWwindow* m_GLFWindow = nullptr;
		std::unique_ptr<GLContext> m_GLContext;

		struct WindowData
		{
			WindowProps		WindowProperties;	// Window Properties
			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}