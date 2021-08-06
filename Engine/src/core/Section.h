// Section.h
// Spontz Demogroup

#pragma once 

#include "core/renderer/Spline.h"

namespace Phoenix {

	enum class SectionType {
		NOT_FOUND,	// No section found

		Loading,

		CameraFPS,
		CameraTarget,
		LightSec,
		ObjectShader,
		ObjectAnimShader,
		DrawImage,
		DrawSkybox,
		DrawVideo,
		DrawQuad,
		DrawFbo,
		DrawScene,
		DrawSceneMatrix,
		DrawSceneMatrixInstanced,
		DrawParticles,
		DrawParticlesImage,
		DrawParticlesScene,
		DrawEmitters,
		DrawEmitterScene,
		SetVariable,
		Sound,
		FboBind,
		FboUnbind,
		EfxAccum,
		EfxBloom,
		EfxBlur,
		EfxFader,
		EfxMotionBlur,
		Test
	};


	class Section {
		friend class demokernel;
		friend class SectionManager;

	public:
		Section();
		virtual ~Section();

	public:
		virtual std::string debug(); // get a debug string. TODO: const function

	private:
		virtual bool load();        // load data from disk
		virtual void init();        // set up local variables before execution
		virtual void exec();        // execute section (normally, render)
		virtual void end();         // destroy allocated memory
		virtual void loadDebugStatic(); // load the debug static string

	public:
		// Blending management
		void EvalBlendingStart();
		void EvalBlendingEnd();

		// Set Render states
		void setRenderStatesStart();
		void setRenderStatesEnd();

	public:
		SectionType                 type;			// section type id
		std::string                 identifier;		// Unique section identifier generated by the demo editor
		std::string                 DataSource;		// origin of this section (for error messages)
		std::string                 type_str;		// Section type (in string format)
		float                       startTime;		// section start time
		float                       endTime;		// section end time
		float                       duration;		// total section seconds
		int32_t                     layer;			// layer where sections run (priority)
		int32_t                     enabled;		// 1: Section enabled; 0: Section disabled
		int32_t                     inited;			// 1: Section inited; 0: Section needs to be inited
		int32_t                     loaded;			// 1: Section loaded; 0: Section not loaded
		int32_t                     ended;			// 1: Section ended; 0: Section not ended
		float                       runTime;		// seconds elapsed since section start
		int32_t                     hasBlend;		// has blending function parameters?
		int32_t                     blendEquation;	// Blend Equation
		int32_t                     sfactor;		// default source blending factor
		int32_t                     dfactor;		// default dest blending factor
		std::vector<float>          param;			// float parameters
		std::vector<std::string>    strings;		// string parameters
		std::vector<std::string>    uniform;		// string of uniform parameters
		std::vector<Spline*>        spline;			// Splines
		std::string                 debugStatic;	// Debug Static string

		// Render states
		bool                        render_drawWireframe;	// Draw in wireframe (disabled by default)
		bool						render_clearDepth;		// Clear depth buffer bit (disabled by default)
		bool						render_clearColor;		// Clear color bit (disabled by default)
		bool						render_disableDepthTest;// Disable depth test (enabled by default)
		bool						render_disableDepthMask;// Disable depth mask writing (enabled by default)

	protected:
		demokernel& m_demo;
	};
}