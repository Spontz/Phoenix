// Window.cpp
// Spontz Demogroup

#include "Main.h"
#include "core/Window.h"
#include "core/platform/windows/WindowsWindow.h"

namespace Phoenix
{
	std::unique_ptr<Window> Window::Create(const WindowProps& props)
	{
		return std::make_unique<WindowsWindow>(props);
	}

}