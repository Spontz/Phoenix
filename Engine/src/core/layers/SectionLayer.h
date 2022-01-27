// SectionLayer.h
// Spontz Demogroup

#pragma once

#include "core/Layer.h"
#include "core/events/DemoKernelEvent.h"
#include "core/events/KeyEvent.h"
#include "core/events/MouseEvent.h"

namespace Phoenix {

	class SectionLayer : public Layer
	{
	public:
		SectionLayer();
		~SectionLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event & e) override;
		
		void Begin();	// Pre-Executing the sections
		void DoExec();	// Execute sections
		void End();		// Post-Executing the sections

		
	private:

	};

}