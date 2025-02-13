// SectionVariableInfo.cpp
// Spontz Demogroup

#include <vector>
#include <string>
#include <functional>
#include "core/SectionManager.h"
#include "SectionVariableInfo.h"

namespace Phoenix {

	const SectionVariable kSectionVariableInfo[] = {
		// Variables common to all secions
		{ SectionType::Global, "t",				SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current time of the section, time is relative to the section" },
		{ SectionType::Global, "tend",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Time when the section ends" },
		{ SectionType::Global, "tdemo",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Global demo time" },
		{ SectionType::Global, "fps",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"FPS of the demo" },
		{ SectionType::Global, "mouseX",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"X value of the mouse position" },
		{ SectionType::Global, "mouseY",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Y value of the mouse position" },
		// Multi-purpose variables
		{ SectionType::Global, "var0",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var1",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var2",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var3",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var4",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var5",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var6",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var7",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var8",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		{ SectionType::Global, "var9",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Multipurpose variable" },
		// Track beats
		{ SectionType::Global, "beat",			SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Sount output beat detection" },
		{ SectionType::Global, "high_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Sum of high frequency sound output" },
		{ SectionType::Global, "mid_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Sum of mid frequency sound output" },
		{ SectionType::Global, "low_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Sum of low frequency sound output" },
		// Camera values
		{ SectionType::Global, "cam_posX",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera position X" },
		{ SectionType::Global, "cam_posY",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera position Y" },
		{ SectionType::Global, "cam_posZ",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera position Z" },
		{ SectionType::Global, "cam_upX",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera up vector X" },
		{ SectionType::Global, "cam_upY",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera up vector Y" },
		{ SectionType::Global, "cam_upZ",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Camera up vector Z" },
		{ SectionType::Global, "cam_yaw",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera yaw" },
		{ SectionType::Global, "cam_pitch",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera pitch" },
		{ SectionType::Global, "cam_roll",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera roll" },
		{ SectionType::Global, "cam_fov",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Current camera field of view" },
		// Graphic constants
		{ SectionType::Global, "vpWidth",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Viewport Width" },
		{ SectionType::Global, "vpHeight",		SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Viewport Height" },
		{ SectionType::Global, "aspectRatio",	SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Viewport Aspect Ratio" },
		// Fbo constants
		{ SectionType::Global, "fbo{}Width",	SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Fbo {} Widht" },
		{ SectionType::Global, "fbo{}Height",	SectionVariableType::EXPRESSION_VARIABLE,		"Expression Variable",	"Fbo {} Height "},
		
		{ SectionType::CameraFPS, "fCameraMode",	SectionVariableType::PARAMETER,				"Parameter",			"Camera Mode (0:Free Camera, 1:Only spline, 2:Only formulas, 3:Spline+Formulas" },
		{ SectionType::CameraFPS, "cCameraFile",	SectionVariableType::SPLINE,				"Spline",				"Spline file that defines the camera movement" },
		{ SectionType::CameraFPS, "PosX",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera position in X axis" },
		{ SectionType::CameraFPS, "PosY",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera position in Y axis" },
		{ SectionType::CameraFPS, "PosZ",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera position in Z axis" },
		{ SectionType::CameraFPS, "UpX",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Up vector in X axis" },
		{ SectionType::CameraFPS, "UpY",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Up vector in Y axis" },
		{ SectionType::CameraFPS, "UpZ",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Up vector in Z axis" },
		{ SectionType::CameraFPS, "Yaw",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Yaw angle" },
		{ SectionType::CameraFPS, "Pitch",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Pitch angle" },
		{ SectionType::CameraFPS, "Roll",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Roll angle" },
		{ SectionType::CameraFPS, "Fov",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Fov angle" },
		{ SectionType::CameraFPS, "Near",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Near plane distance" },
		{ SectionType::CameraFPS, "Far",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Far plane distance" },

		{ SectionType::CameraTarget, "fCameraMode",	SectionVariableType::PARAMETER,				"Parameter",			"Camera Mode (0:Free Camera, 1:Only spline, 2:Only formulas, 3:Spline+Formulas" },
		{ SectionType::CameraTarget, "cCameraFile",	SectionVariableType::SPLINE,				"Spline",				"Spline file that defines the camera movement" },
		{ SectionType::CameraTarget, "PosX",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera position in X axis" },
		{ SectionType::CameraTarget, "PosY",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera position in Y axis" },
		{ SectionType::CameraTarget, "PosZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera position in Z axis" },
		{ SectionType::CameraTarget, "TargetX",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera target in X axis" },
		{ SectionType::CameraTarget, "TargetY",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera target in Y axis" },
		{ SectionType::CameraTarget, "TargetZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Camera target in Z axis" },
		{ SectionType::CameraTarget, "Yaw",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Yaw angle" },
		{ SectionType::CameraTarget, "Pitch",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Pitch angle" },
		{ SectionType::CameraTarget, "Roll",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Roll angle" },
		{ SectionType::CameraTarget, "Fov",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Fov angle" },
		{ SectionType::CameraTarget, "Near",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Near plane distance" },
		{ SectionType::CameraTarget, "Far",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Far plane distance" },

		{ SectionType::DrawEmitterScene, "sScene",				SectionVariableType::STRING,				"String"				"3D model used as reference, each emitter will be placed on the vertex of the given model" },
		{ SectionType::DrawEmitterScene, "fEmissionTime",		SectionVariableType::PARAMETER,				"Parameter",			"Time between emissions (seconds)" },
		{ SectionType::DrawEmitterScene, "fParticleLifetime",	SectionVariableType::PARAMETER,				"Parameter",			"Particles lifetime (seconds)" },
		{ SectionType::DrawEmitterScene, "tx",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System position X" },
		{ SectionType::DrawEmitterScene, "ty",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System position Y" },
		{ SectionType::DrawEmitterScene, "tz",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System position Z" },
		{ SectionType::DrawEmitterScene, "rx",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System rotation X" },
		{ SectionType::DrawEmitterScene, "ry",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System rotation Y" },
		{ SectionType::DrawEmitterScene, "rz",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System rotation Z" },
		{ SectionType::DrawEmitterScene, "sx",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System scale X" },
		{ SectionType::DrawEmitterScene, "sy",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System scale Y" },
		{ SectionType::DrawEmitterScene, "sz",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Particle System scale Z" },
		{ SectionType::DrawEmitterScene, "partSpeed",			SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Speed variability (it impacts the emission time)" },
		{ SectionType::DrawEmitterScene, "partRandomness",		SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Randomness factor on the generated particles (0 to infnite)" },
		{ SectionType::DrawEmitterScene, "velX",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Initial velocity X how particles are emitted" },
		{ SectionType::DrawEmitterScene, "velY",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Initial velocity Y how particles are emitted" },
		{ SectionType::DrawEmitterScene, "velZ",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Initial velocity Z how particles are emitted" },
		{ SectionType::DrawEmitterScene, "forceX",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Force X applied to the particles" },
		{ SectionType::DrawEmitterScene, "forceY",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Force Y applied to the particles" },
		{ SectionType::DrawEmitterScene, "forceZ",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Force Z applied to the particles" },
		{ SectionType::DrawEmitterScene, "colorR",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Color Red applied to the particles" },
		{ SectionType::DrawEmitterScene, "colorG",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Color Green applied to the particles" },
		{ SectionType::DrawEmitterScene, "colorB",				SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Color Blue applied to the particles" },
		{ SectionType::DrawEmitterScene, "nE",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"Current Emitter" },
		{ SectionType::DrawEmitterScene, "TnE",					SectionVariableType::EXPRESSION_CONSTANT,	"Expression Constant",	"Total number of Emitters" },

		{ SectionType::DrawEmitterSceneEx, "TBD",				SectionVariableType::STRING,				"TBD",	"TBD" },
		{ SectionType::DrawEmitterSpline, "TBD",				SectionVariableType::STRING,				"TBD",	"TBD" },
		
		{ SectionType::DrawFbo, "fFboNum",						SectionVariableType::PARAMETER,				"Parameter",			"Framebuffer number to be used" },
		{ SectionType::DrawFbo, "fFboAttachment",				SectionVariableType::PARAMETER,				"Parameter",			"Framebuffer attachment number to be used" },
		{ SectionType::DrawFbo, "fClearScreenBuffer",			SectionVariableType::PARAMETER,				"Parameter",			"Clear the screen buffer before restoring?" },
		{ SectionType::DrawFbo, "fClearDepthBuffer",			SectionVariableType::PARAMETER,				"Parameter",			"Clear the depth buffer before restoring?" },
		
		{ SectionType::DrawFbo2, "fFboNum",						SectionVariableType::PARAMETER,				"Parameter",			"Framebuffer number to be used" },
		{ SectionType::DrawFbo2, "fFboAttachment",				SectionVariableType::PARAMETER,				"Parameter",			"Framebuffer attachment number to be used" },
		{ SectionType::DrawFbo2, "fClearScreenBuffer",			SectionVariableType::PARAMETER,				"Parameter",			"Clear the screen buffer before restoring?" },
		{ SectionType::DrawFbo2, "fClearDepthBuffer",			SectionVariableType::PARAMETER,				"Parameter",			"Clear the depth buffer before restoring?" },
		{ SectionType::DrawFbo2, "fFullscreen",					SectionVariableType::PARAMETER,				"Parameter",			"Draw FBO at fullscreen?" },
		{ SectionType::DrawFbo2, "fFitToContent",				SectionVariableType::PARAMETER,				"Parameter",			"Fit to content (keep aspect ratio) or fill the screen?" },
		{ SectionType::DrawFbo2, "fFilter",						SectionVariableType::PARAMETER,				"Parameter",			"Use bilinear filter?" },
		{ SectionType::DrawFbo2, "tx",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO position X" },
		{ SectionType::DrawFbo2, "ty",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO position Y" },
		{ SectionType::DrawFbo2, "tz",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO position Z" },
		{ SectionType::DrawFbo2, "rx",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO rotation X" },
		{ SectionType::DrawFbo2, "ry",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO rotation Y" },
		{ SectionType::DrawFbo2, "rz",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO rotation Z" },
		{ SectionType::DrawFbo2, "sx",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO scale X" },
		{ SectionType::DrawFbo2, "sy",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO scale Y" },
		{ SectionType::DrawFbo2, "sz",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",	"FBO scale Z" },

		{ SectionType::DrawImage, "fClearScreenBuffer",			SectionVariableType::PARAMETER,				"Parameter",				"Clear the screen buffer before restoring?" },
		{ SectionType::DrawImage, "fClearDepthBuffer",			SectionVariableType::PARAMETER,				"Parameter",				"Clear the depth buffer before restoring?" },
		{ SectionType::DrawImage, "fFullscreen",				SectionVariableType::PARAMETER,				"Parameter",				"Draw Image at fullscreen?" },
		{ SectionType::DrawImage, "fFitToContent",				SectionVariableType::PARAMETER,				"Parameter",				"Fit to content (keep aspect ratio) or fill the screen?" },
		{ SectionType::DrawImage, "fFilter",					SectionVariableType::PARAMETER,				"Parameter",				"Use bilinear filter?" },
		{ SectionType::DrawImage, "tx",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image position X" },
		{ SectionType::DrawImage, "ty",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image position Y" },
		{ SectionType::DrawImage, "tz",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image position Z" },
		{ SectionType::DrawImage, "rx",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image rotation X" },
		{ SectionType::DrawImage, "ry",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image rotation Y" },
		{ SectionType::DrawImage, "rz",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image rotation Z" },
		{ SectionType::DrawImage, "sx",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image scale X" },
		{ SectionType::DrawImage, "sy",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image scale Y" },
		{ SectionType::DrawImage, "sz",							SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Image scale Z" },

		{ SectionType::DrawParticles, "fNumParticles",			SectionVariableType::PARAMETER,				"Parameter",				"Number of particles to generate" },
		{ SectionType::DrawParticles, "tx",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System position X" },
		{ SectionType::DrawParticles, "ty",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System position Y" },
		{ SectionType::DrawParticles, "tz",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System position Z" },
		{ SectionType::DrawParticles, "rx",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System rotation X" },
		{ SectionType::DrawParticles, "ry",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System rotation Y" },
		{ SectionType::DrawParticles, "rz",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System rotation Z" },
		{ SectionType::DrawParticles, "sx",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System scale X" },
		{ SectionType::DrawParticles, "sy",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System scale Y" },
		{ SectionType::DrawParticles, "sz",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System scale Z" },
		{ SectionType::DrawParticles, "nPart",					SectionVariableType::EXPRESSION_CONSTANT,	"Expression Constant",		"Total number of particles" },
		{ SectionType::DrawParticles, "nP",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Current particle number (only changes during section loading time)" },
		{ SectionType::DrawParticles, "px",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial position X" },
		{ SectionType::DrawParticles, "py",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial position Y" },
		{ SectionType::DrawParticles, "pz",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial position Z" },
		{ SectionType::DrawParticles, "cr",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial color R" },
		{ SectionType::DrawParticles, "cg",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial color G" },
		{ SectionType::DrawParticles, "cb",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial color B" },
		{ SectionType::DrawParticles, "ca",						SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial color A" },
		{ SectionType::DrawParticles, "pLife",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particles initial lifetime" },

		{ SectionType::DrawParticlesFbo, "fFboNumber",			SectionVariableType::PARAMETER,				"Parameter",				"FBO Number" },
		{ SectionType::DrawParticlesFbo, "fFboAttachment",		SectionVariableType::PARAMETER,				"Parameter",				"FBO Attachment" },
		{ SectionType::DrawParticlesFbo, "fFitToContent",		SectionVariableType::PARAMETER,				"Parameter",				"Fit to content (keep aspect ratio) or fill the screen?" },
		{ SectionType::DrawParticlesFbo, "fParticlesPerEmitter",SectionVariableType::PARAMETER,				"Parameter",				"Number of particles per emitter (each pixel is considered one emitter)" },
		{ SectionType::DrawParticlesFbo, "fParticleLifetime",	SectionVariableType::PARAMETER,				"Parameter",				"Particles Lifetime" },
		{ SectionType::DrawParticlesFbo, "tx",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System position X" },
		{ SectionType::DrawParticlesFbo, "ty",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System position Y" }, 
		{ SectionType::DrawParticlesFbo, "tz",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System position Z" },
		{ SectionType::DrawParticlesFbo, "rx",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System rotation X" },
		{ SectionType::DrawParticlesFbo, "ry",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System rotation Y" },
		{ SectionType::DrawParticlesFbo, "rz",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System rotation Z" },
		{ SectionType::DrawParticlesFbo, "sx",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System scale X" },
		{ SectionType::DrawParticlesFbo, "sy",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System scale Y" },
		{ SectionType::DrawParticlesFbo, "sz",					SectionVariableType::EXPRESSION_VARIABLE,	"Expression Variable",		"Particle System scale Z" },


	};
	
	const std::size_t kSectionVariableInfoCount = sizeof(kSectionVariableInfo) / sizeof(SectionVariable);
}
