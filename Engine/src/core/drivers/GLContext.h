// GLContext.h
// Spontz Demogroup

#pragma once

#include "core/drivers/GLContext.h"

struct GLFWwindow;

namespace Phoenix {

	class GLContext
	{
	public:
		GLContext(GLFWwindow* windowHandle);

		void Init();
		void SwapBuffers();

		std::string getGLVendor() { return m_GLVendor; };
		std::string getGLRenderer() { return m_GLRenderer; };
		std::string getGLVersion() { return m_GLVersion; };
	private:
		GLFWwindow* m_WindowHandle;

		std::string m_GLVendor;
		std::string m_GLRenderer;
		std::string m_GLVersion;
	};

}