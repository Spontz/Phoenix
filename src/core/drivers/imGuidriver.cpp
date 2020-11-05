// imGuidriver.cpp
// Spontz Demogroup

#include "main.h"

#include "core/drivers/bassdriver.h"
#include "core/drivers/imGuidriver.h"

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
	m_demo(demokernel::GetInstance()),
	p_glfw_window(nullptr),
	m_io(nullptr),
	show_info(true),
	show_fpsHistogram(false),
	show_sesctionInfo(false),
	show_fbo(false),
	show_sound(false),
	show_version(false),
	m_fontScale(1.0f),
	num_fboSetToDraw(0),
	num_fboAttachmentToDraw(0),
	num_fboPerPage(4),
	m_maxRenderFPSScale(60),
	m_currentRenderTime(0)
{
	m_VersionEngine = m_demo.getEngineVersion();
	m_VersionOpenGL = GLDRV->getOpenGLVersion();
	m_VersionGLFW =  GLDRV->getVersion();
	m_VersionBASS = BASSDRV->getVersion();
	m_VersionDyad = m_demo.getLibDyadVersion();
	m_VersionASSIMP = m_demo.getLibAssimpVersion();
	m_VersionImGUI = IMGUI_VERSION;

	for (int i = 0; i < RENDERTIME_SAMPLES; i++)
		m_renderTimes[i] = 0.0f;
}

imGuiDriver::~imGuiDriver()
{
}

void imGuiDriver::init(GLFWwindow *window) {
	// imGUI init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_io = &(ImGui::GetIO()); (void)m_io;
	//m_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//m_io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void imGuiDriver::drawGui()
{
	m_vp = GLDRV->GetFramebufferViewport();

	startDraw();
	{
		if (show_info)
			drawInfo();
		if (show_sesctionInfo)
			drawSesctionInfo();
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

void imGuiDriver::changeFontSize(float baseSize, int width, int height)
{

	m_fontScale = ((float)width * (float)height) / ((640.0f * 400.0f)) * baseSize;
	m_fontScale = m_fontScale/2.5f;

	if (m_fontScale < baseSize)
		m_fontScale = baseSize;
}

void imGuiDriver::drawInfo() {

	// Get Demo status
	char* demoStatus;
	if (m_demo.m_status & DemoStatus::PAUSE) {
		if (m_demo.m_status & DemoStatus::REWIND) demoStatus = stateStr[4];
		else if (m_demo.m_status & DemoStatus::FASTFORWARD) demoStatus = stateStr[5];
		else demoStatus = stateStr[3];

	}
	else {
		if (m_demo.m_status & DemoStatus::REWIND) demoStatus = stateStr[1];
		else if (m_demo.m_status & DemoStatus::FASTFORWARD) demoStatus = stateStr[2];
		else demoStatus = stateStr[0];
	}



	// Draw Menu Bar
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_MenuBar;
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
	if (!ImGui::Begin("Demo Info", &show_info, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::SetWindowFontScale(m_fontScale);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Panels"))
		{
			ImGui::MenuItem("Show Info", "1", &show_info);
			ImGui::MenuItem("Show FPS Histogram", "2", &show_fpsHistogram);
			ImGui::MenuItem("Show FBO's", "3", &show_fbo);
			ImGui::MenuItem("Show section stack", "4", &show_sesctionInfo);
			ImGui::MenuItem("Show sound information", "5", &show_sound);
			ImGui::MenuItem("Show versions", "0", &show_version);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	// Draw Info
		ImGui::SetWindowFontScale(m_fontScale);
		//ImGui::Text("Font: %.3f", m_fontScale);	// Show font size
		ImGui::Text("Fps: %.0f", m_demo.m_fps);
		ImGui::Text("Demo status: %s", demoStatus);
		ImGui::Text("Time: %.2f/%.2f", m_demo.m_demoRunTime, m_demo.m_demoEndTime);
		ImGui::Text("Sound CPU usage: %0.1f%", BASSDRV->getCPUload());
		ImGui::Text("Texture mem used: %.2fmb", m_demo.m_textureManager.mem + m_demo.m_fboManager.mem + m_demo.m_efxBloomFbo.mem + m_demo.m_efxAccumFbo.mem);
		ImGui::Text("Cam Speed: %.3f", m_demo.m_pCamera->MovementSpeed);
		ImGui::Text("Cam Pos: %.1f,%.1f,%.1f", m_demo.m_pCamera->Position.x, m_demo.m_pCamera->Position.y, m_demo.m_pCamera->Position.z);
		ImGui::Text("Cam Front: %.1f,%.1f,%.1f", m_demo.m_pCamera->Front.x, m_demo.m_pCamera->Front.y, m_demo.m_pCamera->Front.z);
		ImGui::Text("Cam Yaw: %.1f, Pitch: %.1f, Roll: %.1f, Zoom: %.1f", m_demo.m_pCamera->Yaw, m_demo.m_pCamera->Pitch, m_demo.m_pCamera->Roll, m_demo.m_pCamera->Zoom);
		if (m_demo.m_slaveMode == 1)
			ImGui::Text("Slave Mode ON");
		else
			ImGui::Text("Slave Mode OFF");
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
	ImGui::Text("ImGUI library version is: %s", m_VersionImGUI.c_str());
	ImGui::End();
}

// Draw the Fbo output
void imGuiDriver::drawFbo() {
	float offsetY = 10; // small offset

	if (num_fboSetToDraw == 0)
		num_fboSetToDraw = 1;

	int fbo_num_min = ((num_fboSetToDraw - 1) * num_fboPerPage);
	int fbo_num_max = (num_fboPerPage - 1) + ((num_fboSetToDraw - 1) * num_fboPerPage);

	if (fbo_num_max >= m_demo.m_fboManager.fbo.size())
		fbo_num_max = static_cast<int>(m_demo.m_fboManager.fbo.size()) - 1;

	ImGui::SetNextWindowPos(ImVec2(0, (2.0f* static_cast<float>(m_vp.y) - offsetY +2.0f* static_cast<float>(m_vp.height)/3.0f)), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width), static_cast<float>(m_vp.height)/3.0f + offsetY), ImGuiCond_Once);
	float fbo_w_size = static_cast<float>(m_vp.width) / 5.0f; // 4 fbo's per row
	float fbo_h_size = static_cast<float>(m_vp.height) / 5.0f; // height is 1/3 screensize

	if(!ImGui::Begin("Fbo info (press '4' to change attachment)", &show_fbo))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
		ImGui::SetWindowFontScale(m_fontScale);
		ImGui::Text("Showing FBO's: %d to %d - Attachment: %d", fbo_num_min, fbo_num_max, num_fboAttachmentToDraw);
		for (int i = fbo_num_min; i <= fbo_num_max; i++) {
			if (i < m_demo.m_fboManager.fbo.size())
			{
				Fbo* my_fbo = m_demo.m_fboManager.fbo[i];
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
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width), 140.0f), ImGuiCond_Once);

	if (!ImGui::Begin("Sound analysis", &show_sound))
	{
		ImGui::End();
		return;
	}
	ImVec2 win = ImGui::GetWindowSize();
	ImGui::SetWindowFontScale(m_fontScale);
	ImGui::Text("Beat value: %.3f, fadeout: %.3f, ratio: %.3f", m_demo.m_beat, m_demo.m_beatFadeout, m_demo.m_beatRatio);
	int plotSamples = BASSDRV->getSpectrumSamples();
	ImGui::Text("Spectrum analyzer: %d samples", plotSamples);
	ImGui::PlotHistogram("", BASSDRV->getSpectrumData(), plotSamples, 0, "Spectrum analyzer", 0.0, 1.0, ImVec2(win.x - 10, win.y - 80)); // For spectrum display
	//ImGui::Text("Waveform display, Displaying 2 channels:");
	//ImGui::PlotHistogram("", BASSDRV->getFFTdata(), plotSamples, 0, "sound waveform", -0.5, 0.5, ImVec2(win.x - 10, win.y - 80)); // For Waveform display

	ImGui::End();
}

// Draws the information of all the sections that are being drawn
void imGuiDriver::drawSesctionInfo()
{
	Section* ds;
	int sec_id;

	ImGui::SetNextWindowPos(ImVec2(2.0f * static_cast<float>(m_vp.width) / 3.0f, 0.0f), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width) / 3.0f, static_cast<float>(m_vp.height + (m_vp.y * 2))), ImGuiCond_Once);

	if (!ImGui::Begin("Section Stack", &show_sesctionInfo))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	ImGui::SetWindowFontScale(m_fontScale);
	for (int i = 0; i < m_demo.m_sectionManager.execSection.size(); i++) {
		sec_id = m_demo.m_sectionManager.execSection[i].second;	// The second value is the ID of the section
		ds = m_demo.m_sectionManager.section[sec_id];
		ImGui::Text(ds->debug().c_str());
		ImGui::Separator();
	}
	ImGui::End();
}

void imGuiDriver::drawFPSHistogram()
{
	m_renderTimes[m_currentRenderTime] = m_demo.m_realFrameTime*1000.f; // Render times in ms

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width), 140.0f), ImGuiCond_Once);

	if (!ImGui::Begin("Render time histogram", &show_fpsHistogram))
	{
		ImGui::End();
		return;
	}
		ImVec2 win = ImGui::GetWindowSize();
		ImGui::SetWindowFontScale(m_fontScale);
		ImGui::DragInt("FPS Scale", &m_maxRenderFPSScale, 10, 10, 1000, "%d");
		float max = 1000.0f / static_cast<float>(m_maxRenderFPSScale);
		ImGui::SameLine();
		ImGui::Text("max (ms): %.2f", max);
		ImGui::PlotLines("", m_renderTimes, RENDERTIME_SAMPLES, m_currentRenderTime, "render time", 0, max, ImVec2(win.x-10, win.y-60));
	ImGui::End();

	m_currentRenderTime++;
	if (m_currentRenderTime >= RENDERTIME_SAMPLES) {
		m_currentRenderTime = 0;
	}
}
