// ImGuiLayer.h
// Spontz Demogroup

#pragma once

#include "core/Layer.h"
#include "core/events/DemoKernelEvent.h"
#include "core/events/KeyEvent.h"
#include "core/events/MouseEvent.h"

#include "core/drivers/ImGuiDriver.h"

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
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
	};

}