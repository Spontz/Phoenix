// ImGuiLayer.cpp
// Spontz Demogroup

#include "Main.h"
#include "core/layers/ImGuiLayer.h"
#include "core/drivers/BassDriver.h"


namespace Phoenix {
	
	static const std::string helpText = \
		"Display information:\n" \
		"--------------------\n" \
		"1         : Show Information (FPS, demo status, time, texture memory used, and other information)\n" \
		"2         : Show FPS Histogram\n" \
		"3         : Show FBO's\n" \
		"4         : Show all FBO's\n" \
		"5         : Show which sections that are being drawn, and some information related to them\n" \
		"6         : Show sound information(spectrum analyzer)\n" \
		"7         : Show Config\n" \
		"9         : Show this help :)\n" \
		"0         : Show engine and libraries versions\n" \
		"F5        : Show Debug screen for networking analysis and simulation\n" \
		"BACKSPACE : Show error Log\n" \
		"ENTER     : Print time on log file\n\n" \
		"Playback control:\n" \
		"-----------------\n" \
		"F1             : PLAY / PAUSE\n" \
		"F2             : REWIND\n" \
		"Left Ctrl + F2 : 1 timeframe REWIND (1/60sec)\n" \
		"F3             : FASTFORWARD\n" \
		"Left Ctrl + F3 : 1 timeframe FAST FORWARD (1/60sec)\n" \
		"F4             : RESTART\n\n" \
		"Camera control:\n" \
		"---------------\n" \
		"Capture camera position : SPACE\n" \
		"Move camera forward     : W\n" \
		"Move camera backwards   : S\n" \
		"Move camera left        : A or LEFT arrow\n" \
		"Move camera right       : D or RIGHT arrow\n" \
		"Move camera up          : UP arrow\n" \
		"Move camera down        : DOWN arrow\n" \
		"Roll camera left        : Q\n" \
		"Roll camera right       : E\n" \
		"Increase camera Speed   : PAGE UP\n" \
		"Decrease camera Speed   : PAGE DOWN\n" \
		"Move camera target      : Mouse movement\n" \
		"Change FOV              : Mouse scroll wheel\n" \
		"Reset camera position   : R";

	ImGuiLayer::ImGuiLayer() :
		Layer("ImGuiLayer"),
		m_demo(*DEMO),
		m_io(nullptr),
		show_log(false),
		show_info(false),
		show_fpsHistogram(false),
		show_sesctionInfo(false),
		show_fbo(false),
		show_fboGrid(false),
		show_sound(false),
		show_version(false),
		show_config(false),
		show_help(false),
		show_debugNet(false),
		m_maxRenderFPSScale(60),
		m_currentRenderTime(0),
		m_expandAllSections(true),
		m_expandAllSectionsChanged(true)
	{
		show_info = m_demo.m_debug;	// if we are on debug, we show the fps info by default
		show_log = m_demo.m_debug; // if we are on debug, the log is opened by default
		m_demoStatus = "";
		m_VersionEngine = m_demo.getEngineVersion();
		m_VersionOpenGL = m_demo.m_Window->getGLVersion();
		m_VendorOpenGL = m_demo.m_Window->getGLVendor();
		m_RendererOpenGL = m_demo.m_Window->getGLRenderer();
		m_VersionGLFW = m_demo.m_Window->getGLFWVersion();
		m_VersionBASS = BASSDRV->getVersion();
		m_VersionDyad = m_demo.getLibDyadVersion();
		m_VersionASSIMP = m_demo.getLibAssimpVersion();
		m_VersionImGUI = IMGUI_VERSION;

		for (int i = 0; i < RENDERTIMES_SAMPLES; i++)
			m_renderTimes[i] = 0.0f;

		// Prepare the text
		m_helpText.appendf(helpText.c_str());

		// Set window properties
		// Window: Fbo
		m_fbo.fboNum = static_cast<int32_t>(m_demo.m_fboManager.fbo.size());
		m_fbo.windowPos = ImVec2(100, 100);
		m_fbo.windowSize = ImVec2(200, 300);

		// Window: Fbo Grid
		m_fboGrid.fboNum = static_cast<int32_t>(m_demo.m_fboManager.fbo.size());
		m_fboGrid.fboColumns = 5;
		m_fboGrid.fboRows = static_cast<int32_t>(ceil(static_cast<float>(m_fboGrid.fboNum) / static_cast<float>(m_fboGrid.fboColumns)));
		m_fboGrid.windowPos = ImVec2(0, 0);
		m_fboGrid.windowSize = ImVec2(200, 300);

		
	}

	void ImGuiLayer::OnAttach()
	{
		PX_PROFILE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		m_io = &(ImGui::GetIO()); (void)m_io;
		m_io->IniFilename = NULL; // Avoid using/saving the ini file
		m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
		//m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
		//m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking	- Experimental (not available yet)
		//m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		// Enable Multi-Viewport / Platform Windows - Experimental (not available yet)
		//m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		/*
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		*/
		SetDarkThemeColors();

		GLFWwindow* window = static_cast<GLFWwindow*>(DEMO->GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Update font size
		changeFontSize();
	}

	void ImGuiLayer::OnDetach()
	{
		PX_PROFILE_FUNCTION();
		// Close ImGui
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
		// Send to the dispatcher the events that ImGUILayer can handle
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(PX_BIND_EVENT_FN(ImGuiLayer::OnKeyPressed));
	}

	void ImGuiLayer::Begin()
	{
		PX_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::OnImGuiRender()
	{
		m_vp = m_demo.m_Window->GetFramebufferViewport();

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
		if (show_fboGrid)
			drawFboGrid();
		if (show_fpsHistogram)
			drawFPSHistogram();
		if (show_sound)
			drawSound();
		if (show_config)
			drawConfig();
		if (show_help)
			drawHelp();
		if (show_debugNet)
			drawDebugNet();
	}

	void ImGuiLayer::End()
	{
		PX_PROFILE_FUNCTION();

		m_io->DisplaySize = ImVec2((float)DEMO->GetWindow().GetWidth(), (float)DEMO->GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Viewports
		/*
		if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		*/
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 0.7f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}
	void ImGuiLayer::drawLog()
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

	void ImGuiLayer::drawInfo()
	{
		// Get Demo status
		if (m_demo.m_status & DemoStatus::PAUSE) {
			if (m_demo.m_status & DemoStatus::REWIND) m_demoStatus = stateStr[4];
			else if (m_demo.m_status & DemoStatus::FASTFORWARD) m_demoStatus = stateStr[5];
			else m_demoStatus = stateStr[3];

		}
		else {
			if (m_demo.m_status & DemoStatus::REWIND) m_demoStatus = stateStr[1];
			else if (m_demo.m_status & DemoStatus::FASTFORWARD) m_demoStatus = stateStr[2];
			else m_demoStatus = stateStr[0];
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
			if (ImGui::BeginMenu("Info Panels"))
			{
				ImGui::MenuItem("Show Log", "BACKSPACE", &show_log);
				ImGui::MenuItem("Show Info", "1", &show_info);
				ImGui::MenuItem("Show FPS Histogram", "2", &show_fpsHistogram);
				ImGui::MenuItem("Show FBO's", "3", &show_fbo);
				ImGui::MenuItem("Show all FBO's", "4", &show_fboGrid);
				ImGui::MenuItem("Show section stack", "5", &show_sesctionInfo);
				ImGui::MenuItem("Show sound information", "6", &show_sound);
				ImGui::MenuItem("Show config", "7", &show_config);
				ImGui::MenuItem("Show help", "9", &show_help);
				ImGui::MenuItem("Show versions", "0", &show_version);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Debug"))
			{
				ImGui::MenuItem("Debug Network", "F5", &show_debugNet);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// Draw Info
		//ImGui::Text("Font: %.3f", m_fontScale);	// Show font size
		ImGui::Text("Fps: %.0f", m_demo.m_fps);
		ImGui::Text("Demo status: %s", m_demoStatus.c_str());
		ImGui::Text("Time: %.2f/%.2f", m_demo.m_demoRunTime, m_demo.m_demoEndTime);
		ImGui::Text("Sound CPU usage: %0.1f%", BASSDRV->getCPUload());
		ImGui::Text("Texture mem used: %.2fmb", m_demo.m_textureManager.m_mem + m_demo.m_fboManager.mem + m_demo.m_efxBloomFbo.mem + m_demo.m_efxAccumFbo.mem);
		if (m_demo.m_slaveMode)
			ImGui::Text("Slave Mode ON");
		else
			ImGui::Text("Slave Mode OFF");

		if (ImGui::TreeNode("Active camera info"))
		{
			drawCameraInfo(m_demo.m_cameraManager.getActiveCamera());
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Internal camera info"))
		{
			drawCameraInfo(m_demo.m_cameraManager.getInternalCamera());
			ImGui::TreePop();
		}

		ImGui::End();
	}

	void ImGuiLayer::drawVersion()
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

	void ImGuiLayer::drawSesctionInfo()
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
				ImGui::SetNextItemOpen(m_expandAllSections);
			if (ImGui::CollapsingHeader(ss.str().c_str())) {
				ImGuiTextBuffer sectionInfoText;
				sectionInfoText.appendf(ds->debug().c_str());
				ImGui::TextUnformatted(sectionInfoText.begin(), sectionInfoText.end());
			}
		}

		m_expandAllSectionsChanged = false;
		ImGui::End();
	}

	void ImGuiLayer::drawFPSHistogram()
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
		ImGui::PlotLines("", m_renderTimes, RENDERTIMES_SAMPLES, m_currentRenderTime, "render time", 0, max, ImVec2(win.x - 10, win.y - 60));
		ImGui::End();

		m_currentRenderTime++;
		if (m_currentRenderTime >= RENDERTIMES_SAMPLES) {
			m_currentRenderTime = 0;
		}
	}

	void ImGuiLayer::drawFbo()
	{
		m_fbo.windowPos = ImVec2(static_cast<float>(m_vp.width)  / 2.0f, static_cast<float>(m_vp.height) / 2.0f);
		m_fbo.windowSize = ImVec2(static_cast<float>(m_vp.width) / 2.0f, static_cast<float>(m_vp.height) / 2.0f);

		// This sets only when the window appears for the first time
		ImGui::SetNextWindowPos(m_fbo.windowPos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(m_fbo.windowSize, ImGuiCond_Appearing);

		if (!ImGui::Begin("Fbo detail", &show_fbo)) {
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// Get window size, in case it has been resized
		m_fbo.windowSize = ImGui::GetWindowSize();

		// Calc Fbo size, cosidering window Padding and space between items (itemSpacng)
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 windowPadding = style.WindowPadding;
		ImVec2 spaceForSpacing(windowPadding.x * 2, windowPadding.y * 2 + 50);
		m_fbo.fboSize.x = m_fbo.windowSize.x - spaceForSpacing.x;
		m_fbo.fboSize.y = m_fbo.windowSize.y - spaceForSpacing.y;

		// Draw Fbo selctor
		if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { decreaseFbo(); }
		ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
		if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { increaseFbo(); }
		ImGui::SameLine();
		ImGui::Text("Fbo: %d", m_fbo.fbo);
		ImGui::SameLine(0, 20);
		
		// Draw Attachment selctor
		if (ImGui::ArrowButton("##left_", ImGuiDir_Left)) { decreaseFboAttachment(m_fbo.fbo); }
		ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
		if (ImGui::ArrowButton("##right_", ImGuiDir_Right)) { increaseFboAttachment(m_fbo.fbo); }
		ImGui::SameLine();
		ImGui::Text("Color Attachment: %d", m_fbo.fboAttachment);
		
		// Draw Fbo
		Fbo* my_fbo = m_demo.m_fboManager.fbo[m_fbo.fbo];
		float aspectRatio = static_cast<float>(my_fbo->width) / static_cast<float>(my_fbo->height);
		m_fbo.fboSize.y = m_fbo.fboSize.x / aspectRatio;
		ImGui::Image((void*)(intptr_t)my_fbo->m_colorAttachment[m_fbo.fboAttachment], m_fbo.fboSize, ImVec2(0, 1), ImVec2(1, 0));
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("Size: %i, %i", my_fbo->width, my_fbo->height);
			ImGui::Text("Format: %s", my_fbo->engineFormat.c_str());
			ImGui::Text("Attachments: %i", my_fbo->numAttachments);
			ImGui::EndTooltip();
		}
		ImGui::End();
	}

	void ImGuiLayer::drawFboGrid()
	{
		m_fboGrid.windowSize = ImVec2(static_cast<float>(m_vp.width), static_cast<float>(m_vp.height));

		// This sets only when the window appears for the first time
		ImGui::SetNextWindowPos(m_fboGrid.windowPos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(m_fboGrid.windowSize, ImGuiCond_Appearing);

		if (!ImGui::Begin("Fbo grid", &show_fboGrid)) {
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// Get window size, in case it has been resized
		m_fboGrid.windowSize = ImGui::GetWindowSize();

		// Calc Fbo size, cosidering window Padding and space between items (itemSpacng)
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 windowPadding = style.WindowPadding;
		ImVec2 itemSpacing = style.ItemInnerSpacing;
		ImVec2 spaceForSpacing(windowPadding.x * 2 + itemSpacing.x * (m_fboGrid.fboColumns - 1), windowPadding.y * 2 + itemSpacing.y * (m_fboGrid.fboRows - 1) + 50);
		m_fboGrid.fboSize.x = (m_fboGrid.windowSize.x - spaceForSpacing.x) / static_cast<float>(m_fboGrid.fboColumns);
		m_fboGrid.fboSize.y = (m_fboGrid.windowSize.y - spaceForSpacing.y) / static_cast<float>(m_fboGrid.fboRows);

		// Draw Attachment selctor
		if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { decreaseFboGridAttachment(); }
		ImGui::SameLine(0.0f, itemSpacing.x);
		if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { increaseFboGridAttachment(); }
		ImGui::SameLine();
		ImGui::Text("Color Attachment: %d", m_fboGrid.fboAttachment);
		
		// Draw Fbo Grid
		int32_t column = 1;
		for (int32_t i = 0; i < m_fboGrid.fboNum; i++) {
			Fbo* my_fbo = m_demo.m_fboManager.fbo[i];
			if (m_fboGrid.fboAttachment < static_cast<int32_t>(my_fbo->numAttachments)) {
				ImGui::Image((void*)(intptr_t)my_fbo->m_colorAttachment[m_fboGrid.fboAttachment], m_fboGrid.fboSize, ImVec2(0, 1), ImVec2(1, 0));
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Fbo Number: %i", i);
					ImGui::Text("Size: %i, %i", my_fbo->width, my_fbo->height);
					ImGui::Text("Format: %s", my_fbo->engineFormat.c_str());
					ImGui::Text("Attachments: %i", my_fbo->numAttachments);
					ImGui::EndTooltip();
				}
			}
			else {
				ImGui::Image((void*)(intptr_t)NULL, m_fboGrid.fboSize);
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Attachment not available");
					ImGui::EndTooltip();
				}
			}
				
			
			if (column < m_fboGrid.fboColumns) {
				ImGui::SameLine(0.0f, itemSpacing.x);
				column++;
			}
			else
				column = 1;
		}
		ImGui::End();
	}

	void ImGuiLayer::drawSound()
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

	void ImGuiLayer::drawConfig()
	{
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width) / 1.75f, 160.0f);
		ImVec2 pos = ImVec2(m_vp.width + m_vp.x - size.x, m_vp.height + m_vp.y - size.y);
		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

		if (!ImGui::Begin("Engine config", &show_config)) {
			ImGui::End();
			return;
		}
		// Font config
		if (ImGui::SliderFloat("Font size", &m_demo.m_debugFontSize, 1, 3, "%.1f")) {
			changeFontSize();
		}
		ImGui::NewLine();

		
		// Grid config
		ImGui::Checkbox("Enable grid", &m_demo.m_debugEnableGrid);
		ImGui::Checkbox("Draw X Axis", &m_demo.m_debugDrawGridAxisX); ImGui::SameLine();
		ImGui::Checkbox("Draw Y Axis", &m_demo.m_debugDrawGridAxisY); ImGui::SameLine();
		ImGui::Checkbox("Draw Z Axis", &m_demo.m_debugDrawGridAxisZ);
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

	void ImGuiLayer::drawHelp()
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

	void ImGuiLayer::drawCameraInfo(Camera* pCamera)
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

	void ImGuiLayer::drawDebugNet()
	{
		ImVec2 pos = ImVec2(0, m_vp.y * 2.0f + 2.0f * (static_cast<float>(m_vp.height) / 3.0f));
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width + (m_vp.x * 2)), static_cast<float>(m_vp.height / 2.0f));


		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);


		if (!ImGui::Begin("Network debugging", &show_debugNet, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
		{
			ImGui::End();
			return;
		}

		if (ImGui::Button("load message")) {
			m_debugMsgFromEditor = Utils::readASCIIFile(m_demo.m_dataFolder + "debug/message.txt");
		}
		ImGui::SameLine();
		if (ImGui::Button("Simulate receive message")) {
			NetDriver::getInstance().processMessage(m_debugMsgFromEditor.c_str());
		}
		ImGui::InputTextMultiline(" ", &m_debugMsgFromEditor, ImVec2(size.x-100, size.y-100));
		
		ImGui::End();
	}

	bool ImGuiLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		bool EventHandled = false;
		uint16_t key = e.GetKeyCode();

		if (m_demo.m_debug) {
			EventHandled = true;
			switch (key) {
			case Key::SHOWLOG:
				show_log = !show_log;
				break;
			case Key::SHOWINFO:
				show_info = !show_info;
				break;
			case Key::SHOWVERSION:
				show_version = !show_version;
				break;
			case Key::SHOWFPSHIST:
				show_fpsHistogram = !show_fpsHistogram;
				break;
			case Key::SHOWFBO:
				show_fbo = !show_fbo;
				break;
			case Key::SHOWALLFBO:
				show_fboGrid = !show_fboGrid;
				break;
			case Key::SHOWSECTIONINFO:
				show_sesctionInfo = !show_sesctionInfo;
				break;
			case Key::SHOWSOUND:
				show_sound = !show_sound;
				break;
			case Key::SHOWCONFIG:
				show_config = !show_config;
				break;
			case Key::SHOWHELP:
				show_help = !show_help;
				break;
			case Key::SHOWDEBUGNET:
				show_debugNet = !show_debugNet;
				break;
			default:
				EventHandled = false;
			}
		}
		return EventHandled;
	}

	void ImGuiLayer::increaseFboGridAttachment()
	{
		m_fboGrid.fboAttachment++;
		if (m_fboGrid.fboAttachment >= FBO_MAX_COLOR_ATTACHMENTS)
			m_fboGrid.fboAttachment = 0;
	}

	void ImGuiLayer::decreaseFboGridAttachment()
	{
		m_fboGrid.fboAttachment--;
		if (m_fboGrid.fboAttachment < 0)
			m_fboGrid.fboAttachment = FBO_MAX_COLOR_ATTACHMENTS-1;
	}

	void ImGuiLayer::increaseFbo()
	{
		m_fbo.fbo++;
		if (m_fbo.fbo >= m_fbo.fboNum)
			m_fbo.fbo = 0;
		// Validate if the selected attachment is valid, if not, we reset to 0
		Fbo* my_fbo = m_demo.m_fboManager.fbo[m_fbo.fbo];
		if (m_fbo.fboAttachment >= static_cast<int32_t>(my_fbo->numAttachments))
			m_fbo.fboAttachment = 0;
	}

	void ImGuiLayer::decreaseFbo()
	{
		m_fbo.fbo--;
		if (m_fbo.fbo < 0)
			m_fbo.fbo = m_fbo.fboNum-1;
		// Validate if the selected attachment is valid, if not, we reset to 0
		Fbo* my_fbo = m_demo.m_fboManager.fbo[m_fbo.fbo];
		if (m_fbo.fboAttachment >= static_cast<int32_t>(my_fbo->numAttachments))
			m_fbo.fboAttachment = 0;
	}

	void ImGuiLayer::increaseFboAttachment(int32_t fbo)
	{
		if ((fbo >= m_fbo.fboNum) || (fbo < 0))
			return;

		uint32_t maxAttach = m_demo.m_fboManager.fbo[fbo]->numAttachments;

		m_fbo.fboAttachment++;
		if (m_fbo.fboAttachment >= static_cast<int32_t>(maxAttach))
			m_fbo.fboAttachment = 0;
	}

	void ImGuiLayer::decreaseFboAttachment(int32_t fbo)
	{
		if ((fbo >= m_fbo.fboNum) || (fbo < 0))
			return;

		uint32_t maxAttach = m_demo.m_fboManager.fbo[fbo]->numAttachments;

		m_fbo.fboAttachment--;
		if (m_fbo.fboAttachment < 0)
			m_fbo.fboAttachment = maxAttach - 1;
	}

	void ImGuiLayer::changeFontSize()
	{
		ImGui::GetIO().FontGlobalScale = m_demo.m_debugFontSize;
	}

	void ImGuiLayer::addLog(std::string_view message)
	{
		m_log.appendf(message.data());
	}

	void ImGuiLayer::clearLog()
	{
		m_log.clear();
	}
}