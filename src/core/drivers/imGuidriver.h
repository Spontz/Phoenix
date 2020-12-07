// imGuiDriver.h
// Spontz Demogroup

#pragma once

#include "main.h"
#include "core/renderer/Viewport.h"

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
	bool	show_grid;


	int32_t			m_numFboSetToDraw;
	uint32_t		m_numFboAttachmentToDraw;
	int32_t			m_numFboPerPage;

	int32_t			m_selectedSection;

	void init(GLFWwindow *window);
	void drawGui();
	void close();
	void changeFontSize(float baseSize, int width, int height);

private:
	std::string m_VersionEngine;
	std::string m_VersionOpenGL;
	std::string m_VendorOpenGL;
	std::string m_RendererOpenGL;
	std::string m_VersionGLFW;
	std::string m_VersionBASS;
	std::string m_VersionDyad;
	std::string m_VersionASSIMP;
	std::string m_VersionImGUI;

	float		m_renderTimes[RENDERTIME_SAMPLES];
	int32_t		m_maxRenderFPSScale;
	int32_t		m_currentRenderTime;
	float		m_fontScale;

	bool		m_expandAllSections;
	bool		m_expandAllSectionsChanged;

	Viewport	m_vp;
	

	const std::string stateStr[6] = {
		"play",
		"play - RW",
		"play - FF",
		"paused",
		"paused - RW",
		"paused - FF"
	};

	void startDraw();
	void endDraw();
	void drawInfo();
	void drawVersion();
	void drawSesctionInfo();
	void drawFPSHistogram();
	void drawFbo();
	void drawSound();
	void drawGridConfig();

};
