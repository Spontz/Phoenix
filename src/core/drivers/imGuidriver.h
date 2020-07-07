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
	GLFWwindow*		p_glfw_window_;
	ImGuiIO*		io_;

public:
	imGuiDriver();
	virtual ~imGuiDriver();

	bool	show_fps;
	bool	show_fpsHistogram;
	bool	show_timing;
	bool	show_sesctionInfo;
	bool	show_fbo;
	bool	show_sound;


	int				num_fboSetToDraw;
	unsigned int	num_fboAttachmentToDraw;
	int				num_fboPerPage;

	void init(GLFWwindow *window);
	void drawGui();
	void close();

private:
	float		renderTimes_[RENDERTIME_SAMPLES];
	int			maxRenderFPSScale_;
	int			currentRenderTime_;
	Viewport	vp_;


	void startDraw();
	void endDraw();
	void drawMenu();
	void drawFps();
	void drawTiming();
	void drawSesctionInfo();
	void drawFPSHistogram();
	void drawFbo();
	void drawSound();

};

#endif
