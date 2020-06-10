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

	void init(GLFWwindow *window);

	void drawGui(bool fps, bool timing, bool sceneInfo, bool fbo);

	void close();

private:

	void startDraw();
	void endDraw();
	void drawSceneInfo();
	void drawFps();
	void drawTiming();
	void drawFbo();

};

#endif
