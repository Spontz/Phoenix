// SectionLayer.h
// Spontz Demogroup

#pragma once

#include "core/Layer.h"
#include "core/SectionManager.h"
#include "core/events/DemoKernelEvent.h"
#include "core/events/KeyEvent.h"
#include "core/events/MouseEvent.h"

namespace Phoenix {

	class SectionLayer : public Layer
	{
	public:
		SectionLayer(SectionManager* SectionManager);
		~SectionLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event & e) override;
		
		void InitSections();	// Initialize and Load Sections
		void ReInitSections();	// Re-Initialize Sections
		void Begin();			// Pre-Executing the sections
		void DoExec();			// Execute sections
		void End();				// Post-Executing the sections

		
	private:
		void ProcessSections(float DemoRunTime);	// Process Sections (sort & prepare sections that need to be rendered and execute them)

		SectionManager* m_SectionManager;
	};

}