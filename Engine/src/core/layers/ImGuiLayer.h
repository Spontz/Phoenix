// ImGuiLayer.h
// Spontz Demogroup

#pragma once

#include "core/Layer.h"
#include "core/events/DemoKernelEvent.h"
#include "core/events/KeyEvent.h"
#include "core/events/MouseEvent.h"

#include "core/renderer/Viewport.h"
#include "core/renderer/Camera.h"

namespace Phoenix {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event & e) override;

		void Begin();
		void OnImGuiRender();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void changeFontSize(float baseSize, int32_t width, int32_t height);
		void addLog(std::string_view message);
		void clearLog();
		void SetDarkThemeColors();

	public:
		bool	show_log;
		bool	show_info;
		bool	show_fpsHistogram;
		bool	show_sesctionInfo;
		bool	show_fbo;
		bool	show_sound;
		bool	show_version;
		bool	show_grid;
		bool	show_help;

		int32_t			m_numFboSetToDraw;
		uint32_t		m_numFboAttachmentToDraw;
		int32_t			m_numFboPerPage;

		int32_t			m_selectedSection;

	private:

		const std::string stateStr[6] = {
			"play",
			"play - RW",
			"play - FF",
			"paused",
			"paused - RW",
			"paused - FF"
		};

		void drawLog();
		void drawInfo();
		void drawVersion();
		void drawSesctionInfo();
		void drawFPSHistogram();
		void drawFbo();
		void drawSound();
		void drawGridConfig();
		void drawHelp();
		void drawCameraInfo(Camera* pCamera);

		bool OnKeyPressed(KeyPressedEvent& e);

	private:
		bool m_BlockEvents = true;
		ImGuiIO* m_io;
		DemoKernel& m_demo;

		std::string m_VersionEngine;
		std::string m_VersionOpenGL;
		std::string m_VendorOpenGL;
		std::string m_RendererOpenGL;
		std::string m_VersionGLFW;
		std::string m_VersionBASS;
		std::string m_VersionDyad;
		std::string m_VersionASSIMP;
		std::string m_VersionImGUI;

		static constexpr uint32_t RENDERTIMES_SAMPLES = 256;

		float		m_renderTimes[RENDERTIMES_SAMPLES];
		int32_t		m_maxRenderFPSScale;
		int32_t		m_currentRenderTime;
		float		m_fontScale;

		bool		m_expandAllSections;
		bool		m_expandAllSectionsChanged;

		ImGuiTextBuffer		m_helpText;

		ImGuiTextBuffer		m_log;

		Viewport	m_vp;
	};

}