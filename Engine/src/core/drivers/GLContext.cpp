// GLContext.cpp
// Spontz Demogroup

#include "Main.h"

#include "core/drivers/GLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Phoenix {

	GLContext::GLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		if (!windowHandle)
			Logger::error("Window Handle is null.");
	}

	void GLContext::Init()
	{
		PX_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		if (!status)
			Logger::error("Failed to initialize Glad.");

		// Store versions
		m_GLFWVersion = glfwGetVersionString();
		m_GLVendor = (const char*)glGetString(GL_VENDOR);
		m_GLRenderer = (const char*)glGetString(GL_RENDERER);
		m_GLVersion = (const char*)glGetString(GL_VERSION);
		// Store OpenGL extensions
		GLint nExt = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &nExt);
		for (GLint i = 0; i < nExt; i++)
		{
			std::string s = (const char*)glGetStringi(GL_EXTENSIONS, i);
			m_GLExtensions.push_back(s);
		}

		//HZ_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Hazel requires at least OpenGL version 4.5!");
	}

	void GLContext::SwapBuffers()
	{
		PX_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}
