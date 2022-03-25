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

		const std::string getGLFWVersion() { return m_GLFWVersion; };
		const std::string getGLVersion() { return m_GLVersion; };
		const std::string getGLVendor() { return m_GLVendor; };
		const std::string getGLRenderer() { return m_GLRenderer; };
		const std::vector<std::string> getGLExtensions() { return m_GLExtensions; };
		
	private:
		GLFWwindow* m_WindowHandle;

		std::string m_GLFWVersion;
		std::string m_GLVendor;
		std::string m_GLRenderer;
		std::string m_GLVersion;
		std::vector<std::string> m_GLExtensions;
	};

}