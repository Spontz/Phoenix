// imGuidriver.h
// Spontz Demogroup

#ifndef IMGUIDRIVER_H
#define IMGUIDRIVER_H

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "core/viewport.h"

#define RENDERTIME_SAMPLES 256

// ******************************************************************

class imGuiDriver {

private:
	GLFWwindow*		p_glfw_window;
	ImGuiIO*		m_io;
	demokernel&		m_demo;

public:
	imGuiDriver();
	virtual ~imGuiDriver();

	bool	show_info;
	bool	show_fpsHistogram;
	bool	show_sesctionInfo;
	bool	show_fbo;
	bool	show_sound;
	bool	show_version;


	int				num_fboSetToDraw;
	unsigned int	num_fboAttachmentToDraw;
	int				num_fboPerPage;

	void init(GLFWwindow *window);
	void drawGui();
	void close();
	void changeFontSize(float baseSize, int width, int height);

private:
	std::string m_VersionEngine;
	std::string m_VersionOpenGL;
	std::string m_VersionGLFW;
	std::string m_VersionBASS;
	std::string m_VersionDyad;
	std::string m_VersionASSIMP;
	std::string m_VersionImGUI;

	float		m_renderTimes[RENDERTIME_SAMPLES];
	int			m_maxRenderFPSScale;
	int			m_currentRenderTime;
	float		m_fontScale;

	Viewport	m_vp;


	void startDraw();
	void endDraw();
	void drawInfo();
	void drawVersion();
	void drawSesctionInfo();
	void drawFPSHistogram();
	void drawFbo();
	void drawSound();

};

#endif
