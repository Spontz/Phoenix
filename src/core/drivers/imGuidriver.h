// imGuidriver.h
// Spontz Demogroup

#ifndef IMGUIDRIVER_H
#define IMGUIDRIVER_H

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "core/viewport.h"



// ******************************************************************

class imGuiDriver {


private:
	GLFWwindow*		p_glfw_window_;
	ImGuiIO*		io_;

public:
	imGuiDriver();
	virtual ~imGuiDriver();

	bool	show_fps;
	bool	show_timing;
	bool	show_sesctionInfo;
	bool	show_fbo;

	int				num_fboSetToDraw;
	unsigned int	num_fboAttachmentToDraw;
	int				num_fboPerPage;

	void init(GLFWwindow *window);
	void drawGui();
	void close();

private:

	void startDraw();
	void endDraw();
	void drawMenu();
	void drawFps();
	void drawTiming();
	void drawSesctionInfo();
	void drawFbo();

};

#endif
