// ImGuiLayer.cpp
// Spontz Demogroup

#include "Main.h"
#include "core/layers/ImGuiLayer.h"

namespace Phoenix {
	
	static const std::string helpText = \
		"Display information:\n" \
		"--------------------\n" \
		"1         : Show Information (FPS, demo status, time, texture memory used, and other information)\n" \
		"2         : Show render time\n" \
		"3         : Show Framebuffer\n" \
		"4         : Show all Framebuffers\n" \
		"5         : Show the render sections being drawn\n" \
		"6         : Show the sound sections being processed\n" \
		"7         : Show sound information(spectrum analyzer)\n" \
		"8         : Show Config\n" \
		"9         : Show this help :)\n" \
		"0         : Show engine and libraries versions\n" \
		"F5        : Show Debug screen for networking analysis and simulation\n" \
		"F6        : Show Debug Memory managers\n" \
		"F8        : Show Menu bar\n" \
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
		show_errorLog(false),
		show_menu(false),
		show_info(false),
		show_renderTime(false),
		show_renderSectionInfo(false),
		show_soundSectionInfo(false),
		show_fbo(false),
		show_fboGrid(false),
		show_sound(false),
		show_version(false),
		show_config(false),
		show_help(false),
		show_debugNet(false),
		show_debugMemory(false),
		
		m_expandAllRenderSections(true),
		m_expandAllRenderSectionsChanged(true),
		m_expandAllSoundSections(true),
		m_expandAllSoundSectionsChanged(true)
	{
		// if we are on debug, we show the menu and generic info panel by default
		if (m_demo.m_debug) {
			show_menu = true;
			show_info = true;
		}		
		
		// Window: Info
		m_info.demoStatus = "";
		m_info.versionEngine = m_demo.getEngineVersion();
		m_info.versionOpenGL = m_demo.m_Window->getGLVersion();
		m_info.vendorOpenGL = m_demo.m_Window->getGLVendor();
		m_info.rendererOpenGL = m_demo.m_Window->getGLRenderer();
		m_info.versionGLFW = m_demo.m_Window->getGLFWVersion();
		m_info.versionMiniAudio = m_demo.m_soundManager.getVersion();
		m_info.versionDyad = m_demo.getLibDyadVersion();
		m_info.versionASSIMP = m_demo.getLibAssimpVersion();
		m_info.versionImGUI = IMGUI_VERSION;

		// Window: Render
		m_render.maxRenderFPSScale = 60;
		m_render.currentRenderTime = 0;
		for (int i = 0; i < m_render.RENDERTIMES_SAMPLES; i++)
			m_render.renderTimes[i] = 0.0f;

		// Prepare the text
		m_helpText.appendf(helpText.c_str());

		// Window: Fbo
		m_fbo.fboNum = static_cast<int32_t>(m_demo.m_fboManager.fbo.size());
		m_fbo.windowPos = ImVec2(100, 100);
		m_fbo.windowSize = ImVec2(200, 300);

		// Window: Fbo Grid
		m_fboGrid.fboNum = static_cast<int32_t>(m_demo.m_fboManager.fbo.size());
		m_fboGrid.fboColumns = 5;
		m_fboGrid.fboRows = static_cast<int32_t>(ceil(static_cast<float>(m_fboGrid.fboNum) / static_cast<float>(m_fboGrid.fboColumns)));
		m_fboGrid.windowPos = ImVec2(0, 20);
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

		if (show_errorLog)
			drawErrorLog();
		if (show_menu)
			drawMenu();
		if (show_info)
			drawInfo();
		if (show_renderSectionInfo)
			drawRenderSectionInfo();
		if (show_soundSectionInfo)
			drawSoundSectionInfo();
		if (show_fbo)
			drawFbo();
		if (show_fboGrid)
			drawFboGrid();
		if (show_renderTime)
			drawRenderTime();
		if (show_sound)
			drawSound();
		if (show_config)
			drawConfig();
		if (show_help)
			drawHelp();
		if (show_debugNet)
			drawDebugNet();
		if (show_debugMemory)
			drawDebugMemory();
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

	void ImGuiLayer::drawErrorLog()
	{
		ImVec2 pos = ImVec2(0, m_vp.y * 2.0f + 2.0f * (static_cast<float>(m_vp.height) / 3.0f));
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width + (m_vp.x * 2)), static_cast<float>(m_vp.height / 3.0f));


		ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Error Log (automatically cleared)", &show_errorLog)) {
			if (ImGui::Button("Clear Log"))	{
				clearErrorLog();
			}
			ImVec2 windowSize = ImGui::GetWindowSize();
			windowSize.x -= 35;
			windowSize.y -= 70;
			ImGui::InputTextMultiline(" ", &m_errorLog, windowSize, ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::End();
	}

	void ImGuiLayer::drawMenu()
	{
		// Draw Menu Bar
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Appearing);
		if (!ImGui::Begin("Demo Menu", &show_menu, window_flags)) {
			ImGui::End();
			return;
		}

		if (ImGui::BeginMenuBar()) {
			ImGui::Text("Spontz (%s)", m_info.versionEngine.c_str());
			ImGui::Separator();
			if (ImGui::BeginMenu("Info Panels")) {
				ImGui::MenuItem("Show this menu", "F8", &show_menu);
				ImGui::MenuItem("Show Errors Log", "BACKSPACE", &show_errorLog);
				ImGui::MenuItem("Show Info", "1", &show_info);
				ImGui::MenuItem("Show render time", "2", &show_renderTime);
				ImGui::MenuItem("Show FBO", "3", &show_fbo);
				ImGui::MenuItem("Show all FBO's", "4", &show_fboGrid);
				ImGui::MenuItem("Show render section stack", "5", &show_renderSectionInfo);
				ImGui::MenuItem("Show sound section stack", "6", &show_soundSectionInfo);
				ImGui::MenuItem("Show sound information", "7", &show_sound);
				ImGui::MenuItem("Show config", "8", &show_config);
				ImGui::MenuItem("Show help", "9", &show_help);
				ImGui::MenuItem("Show versions", "0", &show_version);
				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("Debug")) {
				ImGui::MenuItem("Debug Network", "F5", &show_debugNet);
				ImGui::MenuItem("Debug Nemory", "F6", &show_debugMemory);
				ImGui::EndMenu();
			}

			ImGui::MenuItem("FBO", "3", &show_fbo);
			ImGui::MenuItem("All FBO", "4", &show_fboGrid);
			ImGui::MenuItem("Sections", "5", &show_renderSectionInfo);
			ImGui::MenuItem("Sound", "7", &show_sound);
			ImGui::MenuItem("Config", "8", &show_config);
			ImGui::MenuItem("HELP", "9", &show_help);
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}

	void ImGuiLayer::drawInfo()
	{
		// Get Demo status
		if (m_demo.m_status & DemoStatus::PAUSE) {
			if (m_demo.m_status & DemoStatus::REWIND)
				m_info.demoStatus = "paused - RW";
			else if (m_demo.m_status & DemoStatus::FASTFORWARD)
				m_info.demoStatus = "paused - FF";
			else
				m_info.demoStatus = "paused";

		}
		else {
			if (m_demo.m_status & DemoStatus::REWIND)
				m_info.demoStatus = "play - RW";
			else if (m_demo.m_status & DemoStatus::FASTFORWARD)
				m_info.demoStatus = "play - FF";
			else
				m_info.demoStatus = "play";
		}

		ImGuiWindowFlags window_flags = 0;
		ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f), ImGuiCond_Appearing);
		if (ImGui::Begin("Demo Info", &show_info, window_flags)) {
			// Draw Info
			ImGui::Text("Fps: %.0f", m_demo.m_fps);
			ImGui::Text("Demo status: %s", m_info.demoStatus.c_str());
			ImGui::Text("Time: %.2f/%.2f", m_demo.m_demoRunTime, m_demo.m_demoEndTime);
			ImGui::Text("Texture mem used: %.2fmb", m_demo.m_textureManager.m_mem + m_demo.m_fboManager.mem + m_demo.m_efxBloomFbo.mem + m_demo.m_efxAccumFbo.mem);
			if (m_demo.m_slaveMode)
				ImGui::Text("Slave Mode ON");
			else
				ImGui::Text("Slave Mode OFF");

			if (ImGui::TreeNode("Active camera info")) {
				drawCameraInfo(m_demo.m_cameraManager.getActiveCamera());
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Internal camera info")) {
				drawCameraInfo(m_demo.m_cameraManager.getInternalCamera());
				ImGui::TreePop();
			}

			if (show_version) {
				ImGui::Text("Phoenix engine version: %s", m_info.versionEngine.c_str());
				ImGui::Text("OpenGL driver version: %s", m_info.versionOpenGL.c_str());
				ImGui::Text("OpenGL driver vendor: %s", m_info.vendorOpenGL.c_str());
				ImGui::Text("OpenGL driver renderer: %s", m_info.rendererOpenGL.c_str());
				ImGui::Text("GLFW library version: %s", m_info.versionGLFW.c_str());
				ImGui::Text("MiniAudio library version: %s", m_info.versionMiniAudio.c_str());
				ImGui::Text("Network Dyad.c library version: %s", m_info.versionDyad.c_str());
				ImGui::Text("Assimp library version: %s", m_info.versionASSIMP.c_str());
				ImGui::Text("ImGUI library version: %s", m_info.versionImGUI.c_str());
			}
		}

		ImGui::End();
	}

	void ImGuiLayer::drawRenderSectionInfo()
	{
		const float windowWidth = static_cast<float>(m_vp.width + (m_vp.x * 2)) / 3.0f;
		const float windowHeight = static_cast<float>(m_vp.height + (m_vp.y * 2));

		ImGui::SetNextWindowPos(ImVec2(2.0f * windowWidth, 0.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Render Section Stack", &show_renderSectionInfo, ImGuiWindowFlags_AlwaysHorizontalScrollbar)) {
			 if (ImGui::Checkbox("Expand All", &m_expandAllRenderSections))
				m_expandAllRenderSectionsChanged = true;

			// Render sections
			for (size_t i = 0; i < m_demo.m_sectionManager.m_execRenderSection.size(); i++) {
				const auto sec_id = m_demo.m_sectionManager.m_execRenderSection[i].second;	// The second value is the ID of the section
				const auto ds = m_demo.m_sectionManager.m_section[sec_id];
				std::stringstream ss;
				ss << ds->type_str << " id/layer[" << ds->identifier << "/" + std::to_string(ds->layer) << "]";
				if (m_expandAllRenderSectionsChanged)
					ImGui::SetNextItemOpen(m_expandAllRenderSections);
				if (ImGui::CollapsingHeader(ss.str().c_str())) {
					ImGuiTextBuffer sectionInfoText;
					sectionInfoText.appendf(ds->debug().c_str());
					ImGui::TextUnformatted(sectionInfoText.begin(), sectionInfoText.end());
				}
			}

			m_expandAllRenderSectionsChanged = false;
		}
		ImGui::End();
	}

	void ImGuiLayer::drawSoundSectionInfo()
	{
		const float windowWidth = static_cast<float>(m_vp.width + (m_vp.x * 2)) / 3.0f;
		const float windowHeight = static_cast<float>(m_vp.height + (m_vp.y * 2));

		ImGui::SetNextWindowPos(ImVec2(2.0f * windowWidth, 0.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Sound Section Stack", &show_soundSectionInfo, ImGuiWindowFlags_AlwaysHorizontalScrollbar)) {
			if (ImGui::Checkbox("Expand All", &m_expandAllSoundSections))
				m_expandAllSoundSectionsChanged = true;

			// Sound sections
			for (size_t i = 0; i < m_demo.m_sectionManager.m_execSoundSection.size(); i++) {
				const auto sec_id = m_demo.m_sectionManager.m_execSoundSection[i].second;	// The second value is the ID of the section
				const auto ds = m_demo.m_sectionManager.m_section[sec_id];
				std::stringstream ss;
				ss << ds->type_str << " id/layer[" << ds->identifier << "/" + std::to_string(ds->layer) << "]";
				if (m_expandAllSoundSectionsChanged)
					ImGui::SetNextItemOpen(m_expandAllSoundSections);
				if (ImGui::CollapsingHeader(ss.str().c_str())) {
					ImGuiTextBuffer sectionInfoText;
					sectionInfoText.appendf(ds->debug().c_str());
					ImGui::TextUnformatted(sectionInfoText.begin(), sectionInfoText.end());
				}
			}

			m_expandAllSoundSectionsChanged = false;
		}
		ImGui::End();
	}

	void ImGuiLayer::drawRenderTime()
	{
		m_render.windowSize = ImVec2(static_cast<float>(m_vp.width / 2.f), 180.0f);

		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_vp.x), 20), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(m_render.windowSize, ImGuiCond_FirstUseEver);


		m_render.renderTimes[m_render.currentRenderTime] = m_demo.m_realFrameTime * 1000.f; // Render times in ms

		if (ImGui::Begin("Render time histogram", &show_renderTime)) {
			m_render.windowSize = ImGui::GetWindowSize();

			ImGui::Text("Sections Execution time [Render] (ms): %.5f", m_demo.m_execTime * 1000.0f);
			drawTooltip("Time consumed by all sections being rendered at this moment\n(it does not include initialization or wait times due to vsync)");
			ImGui::Text("Fps: %.0f", m_demo.m_fps);
			ImGui::SetNextItemWidth(80);
			ImGui::DragInt("FPS Scale", &m_render.maxRenderFPSScale, 10, 10, 1000, "%d");
			float max = 1000.0f / static_cast<float>(m_render.maxRenderFPSScale);
			ImGui::SameLine();
			ImGui::Text("Max scale (ms): %.2f", max);
			ImGui::PlotLines(" ", m_render.renderTimes, m_render.RENDERTIMES_SAMPLES, m_render.currentRenderTime, "Frame time", 0, max, ImVec2(m_render.windowSize.x - 10, m_render.windowSize.y - 95));

			m_render.currentRenderTime++;
			if (m_render.currentRenderTime >= m_render.RENDERTIMES_SAMPLES) {
				m_render.currentRenderTime = 0;
			}
		}
		ImGui::End();
	}

	void ImGuiLayer::drawFbo()
	{
		m_fbo.windowPos = ImVec2(static_cast<float>(m_vp.width)  / 2.0f, static_cast<float>(m_vp.height) / 2.0f);
		m_fbo.windowSize = ImVec2(static_cast<float>(m_vp.width) / 2.0f, static_cast<float>(m_vp.height) / 2.0f);

		// This sets only when the window appears for the first time
		ImGui::SetNextWindowPos(m_fbo.windowPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(m_fbo.windowSize, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Fbo detail", &show_fbo)) {
			// Get window size, in case it has been resized
			m_fbo.windowSize = ImGui::GetWindowSize();

			// Calc Fbo size, cosidering window Padding and space between items (itemSpacng)
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec2 windowPadding = style.WindowPadding;
			ImVec2 spaceForSpacing(windowPadding.x * 2, windowPadding.y * 2 + 50);
			m_fbo.fboSize.x = m_fbo.windowSize.x - spaceForSpacing.x;
			m_fbo.fboSize.y = m_fbo.windowSize.y - spaceForSpacing.y;

			// Draw Fbo selctor
			if (ImGui::ArrowButton("##leftFbo", ImGuiDir_Left)) { decreaseFbo(); }
			ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
			if (ImGui::ArrowButton("##rightFbo", ImGuiDir_Right)) { increaseFbo(); }
			ImGui::SameLine();
			ImGui::Text("Fbo: %d", m_fbo.fbo);
			ImGui::SameLine(0, 20);

			// Draw Attachment selctor
			if (ImGui::ArrowButton("##leftAtt", ImGuiDir_Left)) { decreaseFboAttachment(m_fbo.fbo); }
			ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
			if (ImGui::ArrowButton("##rightAtt", ImGuiDir_Right)) { increaseFboAttachment(m_fbo.fbo); }
			ImGui::SameLine();
			ImGui::Text("Color Attachment: %d", m_fbo.fboAttachment);

			// Draw Fbo
			Fbo* my_fbo = m_demo.m_fboManager.fbo[m_fbo.fbo];
			float aspectRatio = static_cast<float>(my_fbo->width) / static_cast<float>(my_fbo->height);
			m_fbo.fboSize.y = m_fbo.fboSize.x / aspectRatio;
			ImGui::Image(my_fbo->m_colorAttachment[m_fbo.fboAttachment], m_fbo.fboSize, ImVec2(0, 1), ImVec2(1, 0));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Size: %i, %i", my_fbo->width, my_fbo->height);
				ImGui::Text("Format: %s", my_fbo->engineFormat.c_str());
				ImGui::Text("Attachments: %i", my_fbo->numAttachments);
				ImGui::EndTooltip();
			}
		}
		ImGui::End();
	}

	void ImGuiLayer::drawFboGrid()
	{
		m_fboGrid.windowSize = ImVec2(static_cast<float>(m_vp.width), static_cast<float>(m_vp.height));

		// This sets only when the window appears for the first time
		ImGui::SetNextWindowPos(m_fboGrid.windowPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(m_fboGrid.windowSize, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Fbo grid", &show_fboGrid)) {

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
					ImGui::Image(my_fbo->m_colorAttachment[m_fboGrid.fboAttachment], m_fboGrid.fboSize, ImVec2(0, 1), ImVec2(1, 0));
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
					ImGui::Image(NULL, m_fboGrid.fboSize);
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
		}
		ImGui::End();
	}

	void ImGuiLayer::drawSound()
	{
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_vp.x), 20), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width), 140.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Sound analysis", &show_sound)) {
			ImVec2 win = ImGui::GetWindowSize();
			int plotSamples = m_demo.m_soundManager.m_spectrogram.getSpectogramSamples();
			ImGui::Text("Spectrum analyzer: %d samples", plotSamples);
			ImGui::PlotHistogram(" ", m_demo.m_soundManager.m_spectrogram.getSpectogramData(), plotSamples, 0, "Spectrum analyzer", 0.0, 1.0, ImVec2(win.x - 10, win.y - 80)); // For spectogram display

			ImGui::Text("Beat: %.3f", m_demo.m_soundManager.m_fBeat);
			ImGui::Text("Frequencies Magnitudes: Low (%.3f), Mid (%.3f), High (%.3f)", m_demo.m_soundManager.m_fLowFreqSum, m_demo.m_soundManager.m_fMidFreqSum, m_demo.m_soundManager.m_fHighFreqSum);
		}
		ImGui::End();
	}

	void ImGuiLayer::drawConfig()
	{
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width) / 1.75f, 190.0f);
		ImVec2 pos = ImVec2(m_vp.width + m_vp.x - size.x, m_vp.height + m_vp.y - size.y);
		ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Engine config", &show_config)) {
			// Font config
			if (ImGui::SliderFloat("Font size", &m_demo.m_debugFontSize, 1, 3, "%.1f")) {
				changeFontSize();
			}
			ImGui::NewLine();


			// Grid config
			ImGui::Checkbox("Enable Axis", &m_demo.m_debugEnableAxis);
			ImGui::Checkbox("Draw X Axis", &m_demo.m_debugDrawAxisX); ImGui::SameLine();
			ImGui::Checkbox("Draw Y Axis", &m_demo.m_debugDrawAxisY); ImGui::SameLine();
			ImGui::Checkbox("Draw Z Axis", &m_demo.m_debugDrawAxisZ);
			ImGui::Checkbox("Enable floor", &m_demo.m_debugEnableFloor);
			if (ImGui::SliderFloat("Floor size", &m_demo.m_pRes->m_gridSize, 1, 50)) {
				m_demo.m_pRes->loadFloor();
			}

			if (ImGui::SliderInt("Floor slices", &m_demo.m_pRes->m_gridSlices, 1, 100)) {
				m_demo.m_pRes->loadFloor();
			}
		}
		ImGui::End();
	}

	void ImGuiLayer::drawHelp()
	{
		ImVec2 size = ImVec2(static_cast<float>(m_vp.width), static_cast<float>(m_vp.height));
		ImVec2 pos = ImVec2(static_cast<float>(m_vp.x), static_cast<float>(m_vp.y));
		ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

		ImGui::SetNextWindowBgAlpha(1.0f);
		if (ImGui::Begin("Help commands", &show_help)) {
			ImGui::TextLinkOpenURL("Link to engine documentation", "https://github.com/Spontz/Phoenix/wiki");
			ImGui::NewLine();
			ImGui::TextUnformatted(m_helpText.begin(), m_helpText.end());
		}
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

		ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Network debugging", &show_debugNet, ImGuiWindowFlags_AlwaysHorizontalScrollbar)) {
			if (ImGui::Button("load message")) {
				m_debugMsgFromEditor = Utils::readASCIIFile(m_demo.m_dataFolder + "debug/message.txt");
			}
			ImGui::SameLine();
			if (ImGui::Button("Simulate receive message")) {
				NetDriver::getInstance().processMessage(m_debugMsgFromEditor.c_str());
			}
			ImGui::InputTextMultiline(" ", &m_debugMsgFromEditor, ImVec2(size.x - 100, size.y - 100));
		}
		ImGui::End();
	}

	void ImGuiLayer::drawDebugMemory()
	{
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_vp.x), static_cast<float>(m_vp.y)), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_vp.width), static_cast<float>(m_vp.height)), ImGuiCond_FirstUseEver);
		
		ImGui::SetNextWindowBgAlpha(1.0f);
		if (ImGui::Begin("Engine internal memory", &show_debugMemory)) {
			static ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

			ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_FittingPolicyDefault_ | ImGuiTabBarFlags_Reorderable;
			if (ImGui::BeginTabBar("MainTabs", tabBarFlags))
			{
				// Global Stats
				if (ImGui::BeginTabItem("Global")) {

					ImGui::SeparatorText("Sections");
					ImGui::Text("Total sections: %d", m_demo.m_sectionManager.m_section.size());
					ImGui::Text("Executing rendering sections: %d", m_demo.m_sectionManager.m_execRenderSection.size());
					ImGui::SeparatorText("Texture memory");
					ImGui::Text("Total textures: %d", m_demo.m_textureManager.texture.size());
					ImGui::Text("Total cubemaps: %d", m_demo.m_textureManager.cubemap.size());
					ImGui::Text("Total FBOs: %d", m_demo.m_fboManager.fbo.size());
					ImGui::Text("Total FBOs (exfAccum): %d", m_demo.m_efxAccumFbo.fbo.size());
					ImGui::Text("Total FBOs (exfBloom): %d", m_demo.m_efxBloomFbo.fbo.size());
					ImGui::Text("Total memory: %.2fMb", m_demo.m_textureManager.m_mem + m_demo.m_fboManager.mem + m_demo.m_efxBloomFbo.mem + m_demo.m_efxAccumFbo.mem);
					ImGui::SeparatorText("3D Models");
					ImGui::Text("Total models: %d", m_demo.m_modelManager.model.size());
					ImGui::SeparatorText("Shaders");
					ImGui::Text("Total shaders: %d", m_demo.m_shaderManager.m_shader.size());
					ImGui::SeparatorText("Videos");
					ImGui::Text("Total videos: %d", m_demo.m_videoManager.VideoMap_.size());
					ImGui::SeparatorText("Sounds");
					ImGui::Text("Total sounds: %d", m_demo.m_soundManager.sound.size());
					ImGui::Text("Executing sound sections: %d", m_demo.m_sectionManager.m_execSoundSection.size());
					ImGui::EndTabItem();
				}

				// Section Manager
				if (ImGui::BeginTabItem("Sections")) {

					ImGui::Text("Total sections: %d", m_demo.m_sectionManager.m_section.size());
					if (ImGui::BeginTable("Sections", 5, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Identifier");
						ImGui::TableSetupColumn("Layer");
						ImGui::TableSetupColumn("Type");
						ImGui::TableSetupColumn("Enabled");
						ImGui::TableSetupColumn("RunTime");
						ImGui::TableHeadersRow();

						for (auto* pSec : m_demo.m_sectionManager.m_section) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text(pSec->identifier.c_str());

							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%d", pSec->layer);

							ImGui::TableSetColumnIndex(2);
							ImGui::Text(pSec->type_str.c_str());

							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%d", pSec->enabled);

							ImGui::TableSetColumnIndex(4);
							ImGui::Text("%.3f", pSec->runTime);
						}
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				// Texture Manager (Textures and Cubemaps)
				if (ImGui::BeginTabItem("Textures")) {
					ImGui::Text("Total mem: %.1fMb", m_demo.m_textureManager.m_mem);
					ImGui::SameLine(0, 40);
					ImGui::Text("Force reload: %d", m_demo.m_textureManager.m_forceLoad);
					ImGui::Text("Total textures: %d", m_demo.m_textureManager.texture.size());
					ImGui::SameLine(0, 40);
					ImGui::Text("Total cubemaps: %d", m_demo.m_textureManager.cubemap.size());
					if (ImGui::BeginTable("Textures", 4, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Texture file");
						ImGui::TableSetupColumn("ID");
						ImGui::TableSetupColumn("Size");
						ImGui::TableSetupColumn("Mem");
						ImGui::TableHeadersRow();

						for (auto pText : m_demo.m_textureManager.texture) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text(pText->m_filename.c_str());

							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%d", pText->m_textureID);

							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%dx%dx%d", pText->m_width, pText->m_height, pText->m_components);

							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%.3fMb", pText->m_mem);
						}
						ImGui::EndTable();
					}

					if (ImGui::BeginTable("Cubemaps", 4, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Cubemap file");
						ImGui::TableSetupColumn("ID");
						ImGui::TableSetupColumn("Size");
						ImGui::TableHeadersRow();

						for (auto pCube : m_demo.m_textureManager.cubemap) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text(pCube->m_filename[0].c_str());

							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%d", pCube->m_cubemapID);

							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%dx%dx6", pCube->m_width[0], pCube->m_height[0]);

							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%.3fMb", pCube->m_mem);
						}
						ImGui::EndTable();
					}
					ImGui::EndTabItem();

				}

				// FBO Manager (fbo's, Bloom and Accum)
				if (ImGui::BeginTabItem("FBO's")) {
					ImGui::Text("FBO mem: %.1fMb", m_demo.m_fboManager.mem);
					ImGui::SameLine(0, 40);
					ImGui::Text("Total FBOs: %d", m_demo.m_fboManager.fbo.size());

					ImGui::Text("EFX Bloom fbo mem: %.1fMb", m_demo.m_efxBloomFbo.mem);
					ImGui::SameLine(0, 40);
					ImGui::Text("Total Bloom fbos: %d", m_demo.m_efxBloomFbo.fbo.size());

					ImGui::Text("EFX Accum fbo mem: %.1fMb", m_demo.m_efxAccumFbo.mem);
					ImGui::SameLine(0, 40);
					ImGui::Text("Total Accum fbos: %d", m_demo.m_efxAccumFbo.fbo.size());

					if (ImGui::BeginTable("FBOs", 4, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("FBO ID");
						ImGui::TableSetupColumn("Format");
						ImGui::TableSetupColumn("Size");
						ImGui::TableSetupColumn("Attachments");
						ImGui::TableHeadersRow();

						int number = 0;
						for (auto* pFbo : m_demo.m_fboManager.fbo) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%d", number);

							ImGui::TableSetColumnIndex(1);
							ImGui::Text(pFbo->engineFormat.c_str());

							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%dx%d", pFbo->width, pFbo->height);

							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%d", pFbo->numAttachments);
							number++;
						}
						ImGui::EndTable();
					}

					if (ImGui::BeginTable("Bloom FBOs", 4, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Bloom FBO ID");
						ImGui::TableSetupColumn("Format");
						ImGui::TableSetupColumn("Size");
						ImGui::TableSetupColumn("Attachments");
						ImGui::TableHeadersRow();

						int number = 0;
						for (auto* pFbo : m_demo.m_efxBloomFbo.fbo) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%d", number);

							ImGui::TableSetColumnIndex(1);
							ImGui::Text(pFbo->engineFormat.c_str());

							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%dx%d", pFbo->width, pFbo->height);

							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%d", pFbo->numAttachments);
							number++;
						}
						ImGui::EndTable();
					}

					if (ImGui::BeginTable("Accum FBOs", 4, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Accum FBO ID");
						ImGui::TableSetupColumn("Format");
						ImGui::TableSetupColumn("Size");
						ImGui::TableSetupColumn("Attachments");
						ImGui::TableHeadersRow();

						int number = 0;
						for (auto* pFbo : m_demo.m_efxAccumFbo.fbo) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%d", number);

							ImGui::TableSetColumnIndex(1);
							ImGui::Text(pFbo->engineFormat.c_str());

							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%dx%d", pFbo->width, pFbo->height);

							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%d", pFbo->numAttachments);
							number++;
						}
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				// Model Manager
				if (ImGui::BeginTabItem("Models")) {
					ImGui::Text("Total models: %d", m_demo.m_modelManager.model.size());

					if (ImGui::BeginTable("Models", 5, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Filename");
						ImGui::TableSetupColumn("Path");
						ImGui::TableSetupColumn("Num Meshes");
						ImGui::TableSetupColumn("Num Vertices");
						ImGui::TableSetupColumn("Num Faces");
						ImGui::TableHeadersRow();

						for (auto pModel : m_demo.m_modelManager.model) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text(pModel->filename.c_str());

							ImGui::TableSetColumnIndex(1);
							ImGui::Text(pModel->directory.c_str());

							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%d", pModel->m_statNumMeshes);

							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%d", pModel->m_statNumVertices);

							ImGui::TableSetColumnIndex(4);
							ImGui::Text("%d", pModel->m_statNumFaces);
						}
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				// Shader Manager
				if (ImGui::BeginTabItem("Shaders")) {
					ImGui::Text("Total shaders: %d", m_demo.m_shaderManager.m_shader.size());

					if (ImGui::BeginTable("Shaders", 2, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Path");
						ImGui::TableSetupColumn("ID");
						ImGui::TableHeadersRow();

						for (auto pShader : m_demo.m_shaderManager.m_shader) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text(pShader->getURI().data());

							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%d", pShader->getId());
						}
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				// Video Manager
				if (ImGui::BeginTabItem("Videos")) {
					ImGui::Text("Total videos: %d", m_demo.m_videoManager.VideoMap_.size());

					if (ImGui::BeginTable("Videos", 1, tableFlags)) {
						// Setup headers
						ImGui::TableSetupColumn("Path");
						ImGui::TableHeadersRow();

						for (auto pVideo : m_demo.m_videoManager.VideoMap_) {
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text(pVideo.first.m_sPath.c_str());
						}
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				// Message Queues Manager
				if (ImGui::BeginTabItem("Editor Event Queues")) {

					ImGui::Text("Events in addition queue: %d", m_demo.m_sectionEventManager.getNumEventsAdditionQueue());
					ImGui::Text("Events in execution queue: %d", m_demo.m_sectionEventManager.getNumEventsExecutionQueue());

					if (ImGui::Button("Clear Log")) {
						clearEventLog();
					}
					ImGui::Text("Processed events (log is automatically cleared):");

					ImVec2 windowSize = ImGui::GetWindowSize();
					windowSize.x -= 30;
					windowSize.y -= 135;
					ImGui::InputTextMultiline(" ", &m_eventLog, windowSize, ImGuiInputTextFlags_ReadOnly);

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
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
				show_errorLog = !show_errorLog;
				break;
			case Key::SHOWMENU:
				show_menu = !show_menu;
				break;
			case Key::SHOWINFO:
				show_info = !show_info;
				break;
			case Key::SHOWVERSION:
				show_version = !show_version;
				break;
			case Key::SHOWRENDERTIME:
				show_renderTime = !show_renderTime;
				break;
			case Key::SHOWFBO:
				show_fbo = !show_fbo;
				break;
			case Key::SHOWALLFBO:
				show_fboGrid = !show_fboGrid;
				break;
			case Key::SHOWRENDERSECTIONINFO:
				show_renderSectionInfo = !show_renderSectionInfo;
				break;
			case Key::SHOWSOUNDSECTIONINFO:
				show_soundSectionInfo = !show_soundSectionInfo;
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
			case Key::SHOWDEBUGMEMORY:
				show_debugMemory = !show_debugMemory;
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

	void ImGuiLayer::drawTooltip(const std::string_view tooltip)
	{
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text(tooltip.data());
			ImGui::EndTooltip();
		}
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

	void ImGuiLayer::addErrorLog(std::string_view message)
	{
		// Prevent infinite log
		if (m_errorLog.size() > 5000)
			m_errorLog.clear();
		m_errorLog.append(message);
	}
	
	void ImGuiLayer::clearErrorLog()
	{
		m_errorLog.clear();
	}

	void ImGuiLayer::addEventLog(std::string_view message)
	{
		// Prevent infinite log
		if (m_eventLog.size() > 1000)
			m_eventLog.clear();
		m_eventLog.append(message);
	}

	void ImGuiLayer::clearEventLog()
	{
		m_eventLog.clear();
	}
		
}