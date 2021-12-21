// WindowsWindow.h
// Spontz Demogroup

#pragma once

#include "core/Window.h"
#include "core/drivers/GLContext.h"

#include <GLFW/glfw3.h>

namespace Phoenix {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_Data.Width; }
		unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual void* GetNativeWindow() const { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		std::unique_ptr<GLContext> m_GLContext;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool DebugMode;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}