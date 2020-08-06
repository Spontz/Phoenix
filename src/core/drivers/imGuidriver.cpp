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
	show_fpsHistogram(false),
	show_timing(true),
	show_sesctionInfo(false),
	show_fbo(false),
	show_sound(false),
	show_version(false),
	fontScale(1.0f),
	num_fboSetToDraw(0),
	num_fboAttachmentToDraw(0),
	num_fboPerPage(4),
	maxRenderFPSScale_(60),
	currentRenderTime_(0)
{
	m_VersionEngine = DEMO->getEngineVersion();
	m_VersionOpenGL = GLDRV->getOpenGLVersion();
	m_VersionGLFW =  GLDRV->getVersion();
	m_VersionBASS = BASSDRV->getVersion();
	m_VersionDyad = DEMO->getLibDyadVersion();
	m_VersionASSIMP = DEMO->getLibAssimpVersion();

	for (int i = 0; i < RENDERTIME_SAMPLES; i++)
		renderTimes_[i] = 0.0f;
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
	vp_ = GLDRV->GetFramebufferViewport();

	startDraw();
	{
		drawMenu();
		if (show_fps)
			drawFps();
		if (show_sesctionInfo)
			drawSesctionInfo();
		if (show_timing)
			drawTiming();
		if (show_version)
			drawVersion();
		if (show_fbo)
			drawFbo();
		if (show_fpsHistogram)
			drawFPSHistogram();
		if (show_sound)
			drawSound();
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
			ImGui::MenuItem("Show FPS", "1", &show_fps);
			ImGui::MenuItem("Show FPS Histogram", "2", &show_fpsHistogram);
			ImGui::MenuItem("Show other Info", "3", &show_timing);
			ImGui::MenuItem("Show FBO's", "4", &show_fbo);
			ImGui::MenuItem("Show section stack", "6", &show_sesctionInfo);
			ImGui::MenuItem("Show sound information", "7", &show_sound);
			ImGui::MenuItem("Show versions", "0", &show_version);
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
		ImGui::SetWindowFontScale(fontScale);
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

void imGuiDriver::drawVersion()
{
	if (!ImGui::Begin("Demo Info"))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	ImGui::Text("Phoenix engine version: %s", m_VersionEngine.c_str());
	ImGui::Text("OpenGL driver version is: %s", m_VersionOpenGL.c_str());
	ImGui::Text("GLFW library version is: %s", m_VersionGLFW.c_str());
	ImGui::Text("Bass library version is: %s", m_VersionBASS.c_str());
	ImGui::Text("Network Dyad.c library version is: %s", m_VersionDyad.c_str());
	ImGui::Text("Assimp library version is: %s", m_VersionASSIMP.c_str());
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
		fbo_num_max = static_cast<int>(DEMO->fboManager.fbo.size()) - 1;

	ImGui::SetNextWindowPos(ImVec2(0, (2.0f* static_cast<float>(vp_.y) - offsetY +2.0f* static_cast<float>(vp_.height)/3.0f)), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(vp_.width), static_cast<float>(vp_.height)/3.0f + offsetY), ImGuiCond_Once);
	float fbo_w_size = static_cast<float>(vp_.width) / 5.0f; // 4 fbo's per row
	float fbo_h_size = static_cast<float>(vp_.height) / 5.0f; // height is 1/3 screensize

	if(!ImGui::Begin("Fbo info (press '5' to change attachment)", &show_fbo))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
		ImGui::SetWindowFontScale(fontScale);
		ImGui::Text("Showing FBO's: %d to %d - Attachment: %d", fbo_num_min, fbo_num_max, num_fboAttachmentToDraw);
		for (int i = fbo_num_min; i <= fbo_num_max; i++) {
			if (i < DEMO->fboManager.fbo.size())
			{
				Fbo* my_fbo = DEMO->fboManager.fbo[i];
				if (num_fboAttachmentToDraw < my_fbo->numAttachments)
					ImGui::Image((void*)(intptr_t)my_fbo->m_colorAttachment[num_fboAttachmentToDraw], ImVec2(fbo_w_size, fbo_h_size), ImVec2(0, 1), ImVec2(1, 0));
				else
					ImGui::Image((void*)(intptr_t)NULL, ImVec2(fbo_w_size, fbo_h_size));
			}
			ImGui::SameLine();
		}
	ImGui::End();
}

void imGuiDriver::drawSound()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(vp_.width), 140.0f), ImGuiCond_Once);

	if (!ImGui::Begin("Sound analysis", &show_sound))
	{
		ImGui::End();
		return;
	}
	ImVec2 win = ImGui::GetWindowSize();
	ImGui::SetWindowFontScale(fontScale);
	ImGui::Text("Beat value: %.3f, fadeout: %.3f, ratio: %.3f", DEMO->beat, DEMO->beat_fadeout, DEMO->beat_ratio);
	ImGui::Text("FFT Samples: %d", FFT_BUFFER_SAMPLES);
	ImGui::PlotHistogram("", BASSDRV->getFFTdata(), FFT_BUFFER_SAMPLES, 0, "sound histogram", 0.0, 1.0, ImVec2(win.x - 10, win.y - 80));
	ImGui::End();
}

// Draws the information of all the sections that are being drawn
void imGuiDriver::drawSesctionInfo()
{
	Section* ds;
	int sec_id;

	ImGui::SetNextWindowPos(ImVec2(2.0f * static_cast<float>(vp_.width) / 3.0f, 0.0f), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(vp_.width) / 3.0f, static_cast<float>(vp_.height + (vp_.y * 2))), ImGuiCond_Once);

	if (!ImGui::Begin("Section Stack", &show_sesctionInfo))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	ImGui::SetWindowFontScale(fontScale);
	for (int i = 0; i < DEMO->sectionManager.execSection.size(); i++) {
		sec_id = DEMO->sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = DEMO->sectionManager.section[sec_id];
		ImGui::Text(ds->debug().c_str());
		ImGui::Separator();
	}
	ImGui::End();
}

void imGuiDriver::drawFPSHistogram()
{
	renderTimes_[currentRenderTime_] = DEMO->realFrameTime*1000.f; // Render times in ms

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(vp_.width), 140.0f), ImGuiCond_Once);

	if (!ImGui::Begin("Render time histogram", &show_fpsHistogram))
	{
		ImGui::End();
		return;
	}
		ImVec2 win = ImGui::GetWindowSize();
		ImGui::SetWindowFontScale(fontScale);
		ImGui::DragInt("FPS Scale", &maxRenderFPSScale_, 10, 10, 1000, "%d");
		float max = 1000.0f / static_cast<float>(maxRenderFPSScale_);
		ImGui::SameLine();
		ImGui::Text("max (ms): %.2f", max);
		ImGui::PlotLines("", renderTimes_, RENDERTIME_SAMPLES, currentRenderTime_, "render time", 0, max, ImVec2(win.x-10, win.y-60));
	ImGui::End();

	currentRenderTime_++;
	if (currentRenderTime_ >= RENDERTIME_SAMPLES) {
		currentRenderTime_ = 0;
	}
}
