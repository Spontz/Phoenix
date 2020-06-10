// imGuidriver.cpp
// Spontz Demogroup

#include "main.h"
#include "imgui.h"
#include "imGuidriver.h"
#include "core/drivers/imGui/imgui_impl_glfw.h"
#include "core/drivers/imGui/imgui_impl_opengl3.h"


// Demo states to show in the drawTiming information ****************
char* stateStr[] = {
	"play",
	"play - RW",
	"play - FF",
	"paused",
	"paused - RW",
	"paused - FF"
};


// imGuiDriver *********************************************************

imGuiDriver::imGuiDriver()
	:
	p_glfw_window_(nullptr),
	io_(nullptr)
{
}

imGuiDriver::~imGuiDriver()
{
}

void imGuiDriver::init(GLFWwindow *window) {
	// imGUI init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io_ = &(ImGui::GetIO()); (void)io_;
	//io_.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io_.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void imGuiDriver::drawGui(bool fps, bool timing, bool sceneInfo, bool fbo)
{
	startDraw();
	{
		if (fps)
			drawFps();
		if (sceneInfo)
			drawSceneInfo();
		if (timing)
			drawTiming();
		if (fbo)
			drawFbo();
	}
	endDraw();
}




void imGuiDriver::startDraw()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void imGuiDriver::endDraw()
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void imGuiDriver::close() {
	// Close ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

// Draws the information of all the scenes that are being drawn
void imGuiDriver::drawSceneInfo()
{
	Section* ds;
	int sec_id;
	
	Viewport vp =  GLDRV->GetFramebufferViewport();
	
	ImGui::SetNextWindowPos(ImVec2(2.0f*(float)vp.width /3.0f, 0.0f), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2((float)vp.width/ 3.0f, (float)(vp.height+(vp.y*2))), ImGuiCond_Once);

	ImGui::Begin("Drawing Stack");
	for (int i = 0; i < DEMO->sectionManager.execSection.size(); i++) {
		sec_id = DEMO->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = DEMO->sectionManager.section[sec_id];
		ImGui::Text(ds->debug().c_str());
		ImGui::Separator();
	}
	ImGui::End();
}

void imGuiDriver::drawFps() {
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
	ImGui::Begin("Demo Info");
		ImGui::Text("Fps: %.0f", DEMO->fps);
	ImGui::End();
}

void imGuiDriver::drawTiming() {
	char* state;
	if (DEMO->state & DEMO_PAUSE) {
		if (DEMO->state & DEMO_REWIND) state = stateStr[4];
		else if (DEMO->state & DEMO_FASTFORWARD) state = stateStr[5];
		else state = stateStr[3];

	}
	else {
		if (DEMO->state & DEMO_REWIND) state = stateStr[1];
		else if (DEMO->state & DEMO_FASTFORWARD) state = stateStr[2];
		else state = stateStr[0];
	}

	ImGui::Begin("Demo Info");
		ImGui::Text("Demo status: %s", state);
		ImGui::Text("Time: %.2f/%.2f", DEMO->runTime, DEMO->endTime);
		ImGui::Text("Sound CPU usage: %0.1f%", BASSDRV->sound_cpu());
		ImGui::Text("Texture mem used: %.2fmb", float(DEMO->textureManager.mem + DEMO->fboManager.mem + DEMO->efxBloomFbo.mem + DEMO->efxAccumFbo.mem));
		ImGui::Text("Cam Speed: %.0f", DEMO->camera->MovementSpeed);
		ImGui::Text("Cam Pos: %.1f,%.1f,%.1f", DEMO->camera->Position.x, DEMO->camera->Position.y, DEMO->camera->Position.z);
		ImGui::Text("Cam Front: %.1f,%.1f,%.1f", DEMO->camera->Front.x, DEMO->camera->Front.y, DEMO->camera->Front.z);
		ImGui::Text("Cam Yaw: %.1f, Pitch: %.1f, Roll: %.1f, Zoom: %.1f", DEMO->camera->Yaw, DEMO->camera->Pitch, DEMO->camera->Roll, DEMO->camera->Zoom);
	ImGui::End();
}

// Draw the Fbo output
void imGuiDriver::drawFbo() {

	int fbo_num_min = ((DEMO->drawFbo - 1) * NUM_FBO_DEBUG);
	int fbo_num_max = (NUM_FBO_DEBUG - 1) + ((DEMO->drawFbo - 1) * NUM_FBO_DEBUG);
	unsigned int fbo_attachment = DEMO->drawFboAttachment;

	Viewport vp = GLDRV->GetFramebufferViewport();


	ImGui::SetNextWindowPos(ImVec2(0, (2.0f*(float)vp.y+2.0f*(float)vp.height/3.0f)), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2((float)(vp.width), (float)vp.height/3.0f), ImGuiCond_Once);
	float fbo_w_size = (float)vp.width / 5.0f; // 4 fbo's per row
	float fbo_h_size = (float)vp.height / 5.0f; // height is 1/3 screensize

	ImGui::Begin("Fbo info");
		ImGui::Text("Showing FBO's: %d to %d - Attachment: %d", fbo_num_min, fbo_num_max, fbo_attachment);
		for (int i = 0; i < NUM_FBO_DEBUG; i++) {
			int fbo_num = (0 + i) + ((DEMO->drawFbo - 1) * NUM_FBO_DEBUG);
			float aspect = (float)DEMO->fboManager.fbo[fbo_num]->width / (float)DEMO->fboManager.fbo[fbo_num]->height;
			if (fbo_attachment < DEMO->fboManager.fbo[fbo_num]->numAttachments)
				ImGui::Image((void*)(intptr_t)DEMO->fboManager.fbo[fbo_num]->colorBufferID[fbo_attachment], ImVec2(fbo_w_size, fbo_h_size),ImVec2(0,1), ImVec2(1,0));
			else
				ImGui::Image((void*)(intptr_t)NULL, ImVec2(fbo_w_size, fbo_h_size));
		
			ImGui::SameLine();
		}
	ImGui::End();
}
