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
		
		void LoadSections();						// Load Sections
		void ReInitSections();						// Re-Initialize Sections
		void ProcessSections(float DemoRunTime);	// Process Sections (sort & prepare sections that need to be rendered and execute them)
		void ExecuteSections(float DemoRuntime);	// Execute sections
		void End();									// Post-Executing the sections

		
	private:
		SectionManager* m_SectionManager;
	};

}