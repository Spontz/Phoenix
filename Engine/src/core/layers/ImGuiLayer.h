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
		struct InfoProperties final {
			std::string demoStatus;
			std::string versionEngine;
			std::string versionOpenGL;
			std::string vendorOpenGL;
			std::string rendererOpenGL;
			std::string versionGLFW;
			std::string versionMiniAudio;
			std::string versionDyad;
			std::string versionASSIMP;
			std::string versionImGUI;
		};

		struct RenderTime final {
			static constexpr uint32_t RENDERTIMES_SAMPLES = 256;

			ImVec2	windowSize;
			float	renderTimes[RENDERTIMES_SAMPLES];
			int32_t	maxRenderFPSScale;
			int32_t	currentRenderTime;
		};

		struct SectionsAndVariables final {
			int32_t	selectedSection = 0;
			std::string	selectedSectionText = "";
			std::vector<SectionInfo>		sectionTypes;		// Stores all different section types available (SectionType ID and Section type String Name)
			std::vector<SectionVariable>	sectionVariables;	// Stores all the possible variables for a given section Type
		};

		struct FboGridProperties final {
			ImVec2	windowPos;
			ImVec2	windowSize;
			ImVec2	fboSize;
			int32_t	fboNum = 0;			// Number of FBO's to draw
			int32_t	fboColumns = 1;		// Columns to display
			int32_t	fboRows = 1;		// Rows to display
			int32_t	fboAttachment = 0;	// Attachmet to display
		};

		struct FboDetailProperties final {
			ImVec2	windowPos;
			ImVec2	windowSize;
			ImVec2	fboSize;
			int32_t	fboNum = 0;			// Number of FBO's
			int32_t	fbo = 0;			// FBO to display
			int32_t	fboAttachment = 0;	// Fbo Attachment to display
		};


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

		void changeFontSize();
		void addErrorLog(std::string_view message);
		void clearErrorLog();
		void addEventLog(std::string_view message);
		void clearEventLog();
		void SetDarkThemeColors();

	public:
		bool	show_errorLog;
		bool	show_menu;
		bool	show_info;
		bool	show_renderTime;
		bool	show_renderSectionInfo;
		bool	show_soundSectionInfo;
		bool	show_fbo;
		bool	show_fboGrid;
		bool	show_sound;
		bool	show_version;
		bool	show_config;
		bool	show_help;
		bool	show_debugNet;
		bool	show_debugMemory;

	private:

		void drawErrorLog();
		void drawMenu();
		void drawInfo();
		void drawRenderSectionInfo();
		void drawSoundSectionInfo();
		void drawRenderTime();
		void drawFbo();
		void drawFboGrid();
		void drawSound();
		void drawConfig();
		void drawHelp();
		void drawCameraInfo(Camera* pCamera);
		void drawDebugNet();
		void drawDebugMemory();

		bool OnKeyPressed(KeyPressedEvent& e);

	private:
		bool m_BlockEvents = true;
		ImGuiIO* m_io;
		DemoKernel& m_demo;
		
		bool		m_expandAllRenderSections;
		bool		m_expandAllRenderSectionsChanged;
		bool		m_expandAllSoundSections;
		bool		m_expandAllSoundSectionsChanged;

		ImGuiTextBuffer		m_helpText;

		std::string			m_errorLog;
		std::string			m_eventLog;

		std::string			m_debugMsgFromEditor;

		Viewport	m_vp;

		// Window Properties
		InfoProperties			m_info;
		RenderTime				m_render;
		FboGridProperties		m_fboGrid;
		FboDetailProperties		m_fbo;

		// Help
		SectionsAndVariables	m_sectionsAndVariables;

		// Static tooltip
		void drawTooltip(const std::string_view tooltip);

		// Fbo detail
		void increaseFbo();
		void decreaseFbo();
		void increaseFboAttachment(int32_t fbo);
		void decreaseFboAttachment(int32_t fbo);

		// Fbo Grid
		void increaseFboGridAttachment();
		void decreaseFboGridAttachment();
	};

}