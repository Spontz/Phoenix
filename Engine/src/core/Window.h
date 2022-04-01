// Window.h
// Window abstraction class
// Spontz Demogroup

#pragma once

#include <sstream>
#include <memory>

#include "Main.h"
#include "core/events/Event.h"

#include "core/drivers/GLContext.h"

#include "core/renderer/Viewport.h"
#include "core/renderer/FboManager.h"

#include <GLFW/glfw3.h>

namespace Phoenix {


#define GL_checkError() DEMO->m_Window.checkError_(__FILE__, __LINE__) 

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

	struct MouseStatus
	{
		bool RightClick;
		bool LeftClick;
		bool MiddleClick;

		float PosX, PosY;
		float LastPosX, LastPosY;

		MouseStatus() :
			RightClick(false),
			LeftClick(false),
			MiddleClick(false),
			PosX(0),
			PosY(0),
			LastPosX(0),
			LastPosY(0)
		{}

	};

	// Viewport management for ExprTK formulas
	struct ViewportExprTK {
		float			Width;
		float			Height;
		float			AspectRatio;
	};

	// The desktop system based Window
	class Window
	{
		friend class DemoKernel;
		friend class SpoReader; // This is needed in order to allow writing the config parameters
		friend class WindowResizeEvent;

	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window();
		~Window();
		bool	Init(std::string const& title);
		void	OnUpdate();
		void	InitRender(bool clear);						// Initialize the render
		bool	checkError_(const char* file, int line);	// Check for errors
		

		uint32_t GetWidth() const { return m_Data.WindowProperties.Width; }
		uint32_t GetHeight() const { return m_Data.WindowProperties.Height; }

		// Fbo configuration
		FboConfig		fboConfig[FBO_BUFFERS];

		// Window attributes
		void SetWindowPos(int x, int y);
		void SetWindowSize(uint32_t width, uint32_t height);
		void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; };
		void SetVSync(bool enabled);
		bool IsVSync();
		glm::vec2 CalcMousePos(glm::vec2 pos);

		// FBO Management
		void	InitFbos();
		 
		// Viewport management
		ViewportExprTK	m_currentViewportExprTK; // Viewport variables for ExprTk
		Viewport		GetFramebufferViewport() const;
		void			SetFramebuffer(); // Unbinds any framebuffer and sets default viewport

		Viewport const& GetCurrentViewport() const;
		void			SetCurrentViewport(Viewport const& viewport);

		void* GetNativeWindow() const { return m_GLFWindow; };
	
		const std::string				getGLFWVersion() { return m_GLContext->getGLFWVersion(); };
		const std::string				getGLVersion() { return m_GLContext->getGLVersion(); };
		const std::string				getGLVendor() { return m_GLContext->getGLVendor(); };
		const std::string				getGLRenderer() { return m_GLContext->getGLRenderer(); };;
		const std::vector<std::string>	getGLExtensions() { return m_GLContext->getGLExtensions(); };;

	private:
		
		void InitOpenGLRenderStates();	// Initialize OpenGL render states
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

		bool OnMouseButtonPressed(uint16_t button);
		bool OnMouseButtonReleased(uint16_t button);
		bool OnMouseMoved(float PosX, float PosY);
		bool OnMouseScrolled(float OffsetX, float OffsetY);
		void OnWindowResize(uint32_t width, uint32_t height);
		void OnProcessInput();
		
	private:
		DemoKernel*					m_demo;
		GLFWwindow*					m_GLFWindow;
		Viewport					m_currentViewport;
		std::unique_ptr<GLContext>	m_GLContext;

		MouseStatus		m_MouseStatus;

		float			m_timeCurrentFrame;
		float			m_timeLastFrame;
		float			m_timeDelta;

		struct WindowData
		{
			WindowProps		WindowProperties;	// Window Properties
			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}