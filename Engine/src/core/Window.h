// Window.h
// Window abstraction class
// Spontz Demogroup

#pragma once

#include <sstream>
#include <memory>

#include "Main.h"
#include "core/events/Event.h"

namespace Phoenix {

	struct WindowProps
	{
		std::string	Title;
		uint32_t	Width;
		uint32_t	Height;
		bool		DebugMode;
		bool		VSync;

		WindowProps(const std::string& title = "Phoenix :: Spontz Demoengine",
			uint32_t width = 640,
			uint32_t height = 480,
			bool debugMode = false,
			bool vSync = true)
			: Title(title), Width(width), Height(height), DebugMode(debugMode), VSync(vSync)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static std::unique_ptr<Window> Create(const WindowProps& props = WindowProps());
	};

}