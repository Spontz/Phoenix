// imGuiDriver.cpp
// Spontz Demogroup

#include "main.h"
#include "core/drivers/BassDriver.h"
#include "core/drivers/ImGuiDriver.h"

namespace Phoenix {

	static const std::string helpText = \
		"Display information:\n" \
		"--------------------\n" \
		"1 Show Information (FPS, demo status, time, texture memory used, and other information)\n" \
		"2 Show FPS Histogram\n" \
		"3 Show FBO's\n" \
		"4 Change FBO attachments to see\n" \
		"5 Show which sections that are being drawn, and some information related to them\n" \
		"6 Show sound information(spectrum analyzer)\n" \
		"7 Show Grid\n" \
		"9 Show this help :)\n" \
		"0 Show engine and libraries versions\n" \
		"BACKSPACE Show error Log\n" \
		"ENTER Print time on log file\n\n" \
		"Playback control:\n" \
		"-----------------\n" \
		"F1 PLAY / PAUSE\n" \
		"F2 REWIND\n" \
		"F3 FASTFORWARD\n" \
		"F4 RESTART\n\n" \
		"Camera control:\n" \
		"---------------\n" \
		"Capture camera position: SPACE\n" \
		"Move camera forward: W\n" \
		"Move camera backwards: S\n" \
		"Move camera left: A\n" \
		"Move camera right: D\n" \
		"Roll camera left: Q\n" \
		"Roll camera right: E\n" \
		"Increase Camera Speed: PAGE UP\n" \
		"Decrease Camera Speed: PAGE DOWN\n" \
		"Change FOV: Mouse scroll wheel\n" \
		"Reset camera position: R";


	ImGuiDriver::ImGuiDriver()
		:
		m_demo(*DEMO),
		p_glfw_window(nullptr),
		m_io(nullptr),
		show_log(false),
		show_info(true),
		show_fpsHistogram(false),
		show_sesctionInfo(false),
		show_fbo(false),
		show_sound(false),
		show_version(false),
		show_grid(false),
		show_help(false),
		m_fontScale(1.0f),
		m_numFboSetToDraw(0),
		m_numFboAttachmentToDraw(0),
		m_numFboPerPage(4),
		m_selectedSection(-1),
		m_maxRenderFPSScale(60),
		m_currentRenderTime(0),
		m_expandAllSections(true),
		m_expandAllSectionsChanged(true)
	{
		show_log = m_demo.m_debug; // if we are on debug, the log is opened by default
		m_VersionEngine = m_demo.getEngineVersion();
		m_VersionOpenGL = GLDRV->getOpenGLVersion();
		m_VendorOpenGL = GLDRV->getOpenGLVendor();
		m_RendererOpenGL = GLDRV->getOpenGLRenderer();
		m_VersionGLFW = GLDRV->getGLFWVersion();
		m_VersionBASS = BASSDRV->getVersion();
		m_VersionDyad = m_demo.getLibDyadVersion();
		m_VersionASSIMP = m_demo.getLibAssimpVersion();
		m_VersionImGUI = IMGUI_VERSION;

		for (int i = 0; i < RENDERTIME_SAMPLES; i++)
			m_renderTimes[i] = 0.0f;

		// Prepare the text
		m_helpText.appendf(helpText.c_str());

	}

	void ImGuiDriver::init(GLFWwindow* window) {
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
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiDriver::drawGui()
	{
		m_vp = GLDRV->GetFramebufferViewport();

		startDraw();
		{
			if (show_log)
				drawLog();
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
			if (show_grid)
				drawGridConfig();
			if (show_help)
				drawHelp();
		}
		endDraw();
	}




	void ImGuiDriver::startDraw()
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiDriver::endDraw()
	{
		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	}

	void ImGuiDriver::drawLog()
	{
		ImVec2 pos = ImVec2(0, m_vp.y * 2.0f + 2.0f * (static_cast<float>(m_vp.height) / 3.0f));
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width + (m_vp.x * 2)), static_cast<float>(m_vp.height / 3.0f));


		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);


		if (!ImGui::Begin("Error Log", &show_log, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
		{
			ImGui::End();
			return;
		}

		if (ImGui::Button("Clear Log"))
		{
			clearLog();
		}
		ImGui::TextUnformatted(m_log.begin(), m_log.end());
		ImGui::End();
	}

	void ImGuiDriver::close() {
		// Close ImGui
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiDriver::changeFontSize(float baseSize, int width, int height)
	{

		m_fontScale = static_cast<float>(width + height) / (1024.0f + 768.0f) * baseSize;

		if (m_fontScale < baseSize)
			m_fontScale = baseSize;

		ImGui::GetIO().FontGlobalScale = m_fontScale;
	}

	void ImGuiDriver::addLog(std::string_view message)
	{
		m_log.appendf(message.data());
	}

	void ImGuiDriver::clearLog()
	{
		m_log.clear();
	}

	void ImGuiDriver::drawCameraInfo(Camera* pCamera)
	{
		if (pCamera != nullptr) {
			ImGui::Text("  Name: %s, Type: %d", pCamera->TypeStr.c_str(), pCamera->Type);
			ImGui::Text("  Speed: %.3f", pCamera->getMovementSpeed());
			glm::vec3 camPosition = pCamera->getPosition();
			ImGui::Text("  Pos: %.1f,%.1f,%.1f", camPosition.x, camPosition.y, camPosition.z);
			glm::vec3 camTarget = pCamera->getTarget();
			ImGui::Text("  Target: %.1f,%.1f,%.1f", camTarget.x, camTarget.y, camTarget.z);
			glm::vec3 camFront = pCamera->getFront();
			ImGui::Text("  Front: %.1f,%.1f,%.1f", camFront.x, camFront.y, camFront.z);
			ImGui::Text("  Yaw: %.1f, Pitch: %.1f, Roll: %.1f, Fov: %.1f", pCamera->getYaw(), pCamera->getPitch(), pCamera->getRoll(), pCamera->getFov());
			ImGui::Text("  Frustum Near: %.1f, Far: %.1f", pCamera->getFrustumNear(), pCamera->getFrustumFar());
		}
	}

	void ImGuiDriver::drawInfo()
	{
		// Get Demo status
		std::string demoStatus;
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
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Appearing);
		if (!ImGui::Begin("Demo Info", &show_info, window_flags))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Panels"))
			{
				ImGui::MenuItem("Show Log", "BACKSPACE", &show_log);
				ImGui::MenuItem("Show Info", "1", &show_info);
				ImGui::MenuItem("Show FPS Histogram", "2", &show_fpsHistogram);
				ImGui::MenuItem("Show FBO's", "3", &show_fbo);
				ImGui::MenuItem("Show section stack", "5", &show_sesctionInfo);
				ImGui::MenuItem("Show sound information", "6", &show_sound);
				ImGui::MenuItem("Show grid panel", "7", &show_grid);
				ImGui::MenuItem("Show help", "9", &show_help);
				ImGui::MenuItem("Show versions", "0", &show_version);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// Draw Info
		//ImGui::Text("Font: %.3f", m_fontScale);	// Show font size
		ImGui::Text("Fps: %.0f", m_demo.m_fps);
		ImGui::Text("Demo status: %s", demoStatus.c_str());
		ImGui::Text("Time: %.2f/%.2f", m_demo.m_demoRunTime, m_demo.m_demoEndTime);
		ImGui::Text("Sound CPU usage: %0.1f%", BASSDRV->getCPUload());
		ImGui::Text("Texture mem used: %.2fmb", m_demo.m_textureManager.mem + m_demo.m_fboManager.mem + m_demo.m_efxBloomFbo.mem + m_demo.m_efxAccumFbo.mem);
		if (m_demo.m_slaveMode)
			ImGui::Text("Slave Mode ON");
		else
			ImGui::Text("Slave Mode OFF");

		if (ImGui::TreeNode("Active camera info"))
		{
			drawCameraInfo(m_demo.m_pActiveCamera);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Default camera info"))
		{
			drawCameraInfo(m_demo.m_pDefaultCamera);
			ImGui::TreePop();
		}

		ImGui::End();
	}

	void ImGuiDriver::drawVersion()
	{
		if (!ImGui::Begin("Demo Info")) {
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}
		ImGui::Text("Phoenix engine version: %s", m_VersionEngine.c_str());
		ImGui::Text("OpenGL driver version: %s", m_VersionOpenGL.c_str());
		ImGui::Text("OpenGL driver vendor: %s", m_VendorOpenGL.c_str());
		ImGui::Text("OpenGL driver renderer: %s", m_RendererOpenGL.c_str());
		ImGui::Text("GLFW library version: %s", m_VersionGLFW.c_str());
		ImGui::Text("Bass library version: %s", m_VersionBASS.c_str());
		ImGui::Text("Network Dyad.c library version: %s", m_VersionDyad.c_str());
		ImGui::Text("Assimp library version: %s", m_VersionASSIMP.c_str());
		ImGui::Text("ImGUI library version: %s", m_VersionImGUI.c_str());
		ImGui::End();
	}

	// Draw the Fbo output
	void ImGuiDriver::drawFbo()
	{
		constexpr float offsetY = 10.0f; // small offset

		if (m_numFboSetToDraw == 0)
			m_numFboSetToDraw = 1;

		const auto fboNumMin = ((m_numFboSetToDraw - 1) * m_numFboPerPage);
		int32_t fboNumMax = (m_numFboPerPage - 1) + ((m_numFboSetToDraw - 1) * m_numFboPerPage);

		if (fboNumMax >= static_cast<int>(m_demo.m_fboManager.fbo.size()))
			fboNumMax = static_cast<int>(m_demo.m_fboManager.fbo.size()) - 1;

		ImGui::SetNextWindowPos(ImVec2(0, (2.0f * static_cast<float>(m_vp.y) - offsetY + 2.0f * static_cast<float>(m_vp.height) / 3.0f)), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width), static_cast<float>(m_vp.height) / 3.0f + offsetY), ImGuiCond_Appearing);
		const float fbo_w_size = static_cast<float>(m_vp.width) / 5.0f; // 4 fbo's per row
		const float fbo_h_size = static_cast<float>(m_vp.height) / 5.0f; // height is 1/3 screensize

		if (!ImGui::Begin("Fbo info (press '4' to change attachment)", &show_fbo)) {
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		ImGui::Text("Showing FBO's: %d to %d - Attachment: %d", fboNumMin, fboNumMax, m_numFboAttachmentToDraw);
		for (size_t i = fboNumMin; i <= fboNumMax; ++i) {
			if (i < m_demo.m_fboManager.fbo.size()) {
				Fbo* my_fbo = m_demo.m_fboManager.fbo[i];
				if (m_numFboAttachmentToDraw < my_fbo->numAttachments)
					ImGui::Image((void*)(intptr_t)my_fbo->m_colorAttachment[m_numFboAttachmentToDraw], ImVec2(fbo_w_size, fbo_h_size), ImVec2(0, 1), ImVec2(1, 0));
				else
					ImGui::Image((void*)(intptr_t)NULL, ImVec2(fbo_w_size, fbo_h_size));
			}
			ImGui::SameLine();
		}
		ImGui::End();
	}

	void ImGuiDriver::drawSound()
	{
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_vp.x), 0), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width), 140.0f), ImGuiCond_Appearing);

		if (!ImGui::Begin("Sound analysis", &show_sound)) {
			ImGui::End();
			return;
		}
		ImVec2 win = ImGui::GetWindowSize();
		int plotSamples = BASSDRV->getSpectrumSamples();
		ImGui::Text("Spectrum analyzer: %d samples", plotSamples);
		ImGui::PlotHistogram("", BASSDRV->getSpectrumData(), plotSamples, 0, "Spectrum analyzer", 0.0, 1.0, ImVec2(win.x - 10, win.y - 80)); // For spectrum display
		//ImGui::Text("Waveform display, Displaying 2 channels:");
		//ImGui::PlotHistogram("", BASSDRV->getFFTdata(), plotSamples, 0, "sound waveform", -0.5, 0.5, ImVec2(win.x - 10, win.y - 80)); // For Waveform display

		ImGui::End();
	}

	void ImGuiDriver::drawGridConfig()
	{
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width) / 1.75f, 160.0f);
		ImVec2 pos = ImVec2(m_vp.width + m_vp.x - size.x, m_vp.height + m_vp.y - size.y);
		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

		if (!ImGui::Begin("Grid config", &show_grid)) {
			ImGui::End();
			return;
		}
		ImGui::Checkbox("Enable grid", &m_demo.m_debug_drawGrid);
		ImGui::NewLine();
		ImGui::Checkbox("Draw X Axis", &m_demo.m_debug_drawGridAxisX); ImGui::SameLine();
		ImGui::Checkbox("Draw Y Axis", &m_demo.m_debug_drawGridAxisY); ImGui::SameLine();
		ImGui::Checkbox("Draw Z Axis", &m_demo.m_debug_drawGridAxisZ);
		if (ImGui::SliderFloat("Size", &m_demo.m_pRes->m_gridSize, 0, 50, "%.1f")) {
			if (m_demo.m_pRes->m_gridSize < 0)
				m_demo.m_pRes->m_gridSize = 0;
			if (m_demo.m_pRes->m_gridSize > 50)
				m_demo.m_pRes->m_gridSize = 50;
			m_demo.m_pRes->loadGrid();
		}

		if (ImGui::SliderInt("Slices", &m_demo.m_pRes->m_gridSlices, 1, 100)) {
			if (m_demo.m_pRes->m_gridSlices < 1)
				m_demo.m_pRes->m_gridSlices = 1;
			if (m_demo.m_pRes->m_gridSlices > 100)
				m_demo.m_pRes->m_gridSlices = 100;
			m_demo.m_pRes->loadGrid();
		}

		ImGui::End();
	}

	void ImGuiDriver::drawHelp()
	{
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width), static_cast<float>(m_vp.height));
		ImVec2 pos = ImVec2(static_cast<float>(m_vp.x), static_cast<float>(m_vp.y));
		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

		if (!ImGui::Begin("Help commands", &show_help)) {
			ImGui::End();
			return;
		}
		ImGui::TextUnformatted(m_helpText.begin(), m_helpText.end());
		ImGui::End();
	}

	// Draws the information of all the sections that are being drawn
	void ImGuiDriver::drawSesctionInfo()
	{
		const float windowWidth = static_cast<float>(m_vp.width + (m_vp.x * 2)) / 3.0f;
		const float windowHeight = static_cast<float>(m_vp.height + (m_vp.y * 2));

		ImGui::SetNextWindowPos(ImVec2(2.0f * windowWidth, 0.0f), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Appearing);

		if (!ImGui::Begin(
			"Section Stack",
			&show_sesctionInfo,
			ImGuiWindowFlags_AlwaysHorizontalScrollbar
		)) {
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		if (ImGui::Checkbox("Expand All", &m_expandAllSections))
			m_expandAllSectionsChanged = true;

		for (size_t i = 0; i < m_demo.m_sectionManager.m_execSection.size(); i++) {
			const auto sec_id = m_demo.m_sectionManager.m_execSection[i].second;	// The second value is the ID of the section
			const auto ds = m_demo.m_sectionManager.m_section[sec_id];
			std::stringstream ss;
			ss << ds->type_str << " id/layer[" << ds->identifier << "/" + std::to_string(ds->layer) << "]";
			if (m_expandAllSectionsChanged)
				ImGui::SetNextTreeNodeOpen(m_expandAllSections);
			if (ImGui::CollapsingHeader(ss.str().c_str())) {
				ImGuiTextBuffer sectionInfoText;
				sectionInfoText.appendf(ds->debug().c_str());
				ImGui::TextUnformatted(sectionInfoText.begin(), sectionInfoText.end());
			}
		}

		m_expandAllSectionsChanged = false;
		ImGui::End();
	}

	void ImGuiDriver::drawFPSHistogram()
	{
		m_renderTimes[m_currentRenderTime] = m_demo.m_realFrameTime * 1000.f; // Render times in ms

		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_vp.x), 0), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width / 2.0f), 140.0f), ImGuiCond_Appearing);

		if (!ImGui::Begin("Render time histogram", &show_fpsHistogram))
		{
			ImGui::End();
			return;
		}
		ImVec2 win = ImGui::GetWindowSize();
		ImGui::DragInt("FPS Scale", &m_maxRenderFPSScale, 10, 10, 1000, "%d");
		float max = 1000.0f / static_cast<float>(m_maxRenderFPSScale);
		ImGui::SameLine();
		ImGui::Text("max (ms): %.2f", max);
		ImGui::PlotLines("", m_renderTimes, RENDERTIME_SAMPLES, m_currentRenderTime, "render time", 0, max, ImVec2(win.x - 10, win.y - 60));
		ImGui::End();

		m_currentRenderTime++;
		if (m_currentRenderTime >= RENDERTIME_SAMPLES) {
			m_currentRenderTime = 0;
		}
	}
}