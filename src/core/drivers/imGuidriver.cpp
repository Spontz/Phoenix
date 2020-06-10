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
	io_(nullptr),
	show_fps(true),
	show_timing(true),
	show_sesctionInfo(false),
	show_fbo(false),
	num_fboSetToDraw(0),
	num_fboAttachmentToDraw(0),
	num_fboPerPage(4)
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

void imGuiDriver::drawGui()
{
	startDraw();
	{
		drawMenu();
		if (show_fps)
			drawFps();
		if (show_sesctionInfo)
			drawSesctionInfo();
		if (show_timing)
			drawTiming();
		if (show_fbo)
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

void imGuiDriver::drawMenu() {

	ImGuiWindowFlags window_flags = 0;

	window_flags |= ImGuiWindowFlags_MenuBar;

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
	if (!ImGui::Begin("Demo Info",false, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Demo"))
		{
			ImGui::MenuItem("Show FPS", "Y", &show_fps);
			ImGui::MenuItem("Show other Info", "T", &show_timing);
			ImGui::MenuItem("Show FBO's", "F", &show_fbo);
			ImGui::MenuItem("Show section stack", "U", &show_sesctionInfo);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();
}

void imGuiDriver::drawFps() {
	if (!ImGui::Begin("Demo Info"))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
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

	if (!ImGui::Begin("Demo Info"))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
		ImGui::Text("Demo status: %s", state);
		ImGui::Text("Time: %.2f/%.2f", DEMO->runTime, DEMO->endTime);
		ImGui::Text("Sound CPU usage: %0.1f%", BASSDRV->sound_cpu());
		ImGui::Text("Texture mem used: %.2fmb", DEMO->textureManager.mem + DEMO->fboManager.mem + DEMO->efxBloomFbo.mem + DEMO->efxAccumFbo.mem);
		ImGui::Text("Cam Speed: %.0f", DEMO->camera->MovementSpeed);
		ImGui::Text("Cam Pos: %.1f,%.1f,%.1f", DEMO->camera->Position.x, DEMO->camera->Position.y, DEMO->camera->Position.z);
		ImGui::Text("Cam Front: %.1f,%.1f,%.1f", DEMO->camera->Front.x, DEMO->camera->Front.y, DEMO->camera->Front.z);
		ImGui::Text("Cam Yaw: %.1f, Pitch: %.1f, Roll: %.1f, Zoom: %.1f", DEMO->camera->Yaw, DEMO->camera->Pitch, DEMO->camera->Roll, DEMO->camera->Zoom);
	ImGui::End();
}

// Draw the Fbo output
void imGuiDriver::drawFbo() {
	float offsetY = 10; // small offset

	if (num_fboSetToDraw == 0)
		num_fboSetToDraw = 1;

	int fbo_num_min = ((num_fboSetToDraw - 1) * num_fboPerPage);
	int fbo_num_max = (num_fboPerPage - 1) + ((num_fboSetToDraw - 1) * num_fboPerPage);

	if (fbo_num_max >= DEMO->fboManager.fbo.size())
		fbo_num_max = DEMO->fboManager.fbo.size() - 1;

	Viewport vp = GLDRV->GetFramebufferViewport();


	ImGui::SetNextWindowPos(ImVec2(0, (2.0f*(float)(vp.y) - offsetY +2.0f*(float)vp.height/3.0f)), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2((float)(vp.width), (float)vp.height/3.0f + offsetY), ImGuiCond_Once);
	float fbo_w_size = (float)vp.width / 5.0f; // 4 fbo's per row
	float fbo_h_size = (float)vp.height / 5.0f; // height is 1/3 screensize

	if(!ImGui::Begin("Fbo info (press 'G' to change attachment)"))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
		ImGui::Text("Showing FBO's: %d to %d - Attachment: %d", fbo_num_min, fbo_num_max, num_fboAttachmentToDraw);
		for (int i = 0; i < num_fboPerPage; i++) {
			int fbo_num = (0 + i) + ((num_fboSetToDraw - 1) * num_fboPerPage);
			if (fbo_num < DEMO->fboManager.fbo.size())
			{
				float aspect = (float)DEMO->fboManager.fbo[fbo_num]->width / (float)DEMO->fboManager.fbo[fbo_num]->height;
				if (num_fboAttachmentToDraw < DEMO->fboManager.fbo[fbo_num]->numAttachments)
					ImGui::Image((void*)(intptr_t)DEMO->fboManager.fbo[fbo_num]->colorBufferID[num_fboAttachmentToDraw], ImVec2(fbo_w_size, fbo_h_size), ImVec2(0, 1), ImVec2(1, 0));
				else
					ImGui::Image((void*)(intptr_t)NULL, ImVec2(fbo_w_size, fbo_h_size));
			}
			ImGui::SameLine();
		}
	ImGui::End();
}

// Draws the information of all the sections that are being drawn
void imGuiDriver::drawSesctionInfo()
{
	Section* ds;
	int sec_id;

	Viewport vp = GLDRV->GetFramebufferViewport();

	ImGui::SetNextWindowPos(ImVec2(2.0f * (float)vp.width / 3.0f, 0.0f), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2((float)vp.width / 3.0f, (float)(vp.height + (vp.y * 2))), ImGuiCond_Once);

	if (!ImGui::Begin("Section Stack"))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	for (int i = 0; i < DEMO->sectionManager.execSection.size(); i++) {
		sec_id = DEMO->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = DEMO->sectionManager.section[sec_id];
		ImGui::Text(ds->debug().c_str());
		ImGui::Separator();
	}
	ImGui::End();
}
