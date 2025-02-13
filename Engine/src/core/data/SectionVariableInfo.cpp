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
		{ SectionType::Global, "t",				SectionVariableType::EXPRESSION_VARIABLE,	"Current time of the section, time is relative to the section" },
		{ SectionType::Global, "tend",			SectionVariableType::EXPRESSION_VARIABLE,	"Time when the section ends" },
		{ SectionType::Global, "tdemo",			SectionVariableType::EXPRESSION_VARIABLE,	"Global demo time" },
		{ SectionType::Global, "fps",			SectionVariableType::EXPRESSION_VARIABLE,	"FPS of the demo" },
		{ SectionType::Global, "mouseX",		SectionVariableType::EXPRESSION_VARIABLE,	"X value of the mouse position" },
		{ SectionType::Global, "mouseY",		SectionVariableType::EXPRESSION_VARIABLE,	"Y value of the mouse position" },
		// Multi-purpose variables
		{ SectionType::Global, "var0",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var1",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var2",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var3",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var4",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var5",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var6",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var7",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var8",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		{ SectionType::Global, "var9",			SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" },
		// Track beats
		{ SectionType::Global, "beat",			SectionVariableType::EXPRESSION_VARIABLE,	"Sount output beat detection" },
		{ SectionType::Global, "high_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,	"Sum of high frequency sound output" },
		{ SectionType::Global, "mid_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,	"Sum of mid frequency sound output" },
		{ SectionType::Global, "low_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,	"Sum of low frequency sound output" },
		// Camera values
		{ SectionType::Global, "cam_posX",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera position X" },
		{ SectionType::Global, "cam_posY",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera position Y" },
		{ SectionType::Global, "cam_posZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera position Z" },
		{ SectionType::Global, "cam_upX",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera up vector X" },
		{ SectionType::Global, "cam_upY",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera up vector Y" },
		{ SectionType::Global, "cam_upZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera up vector Z" },
		{ SectionType::Global, "cam_yaw",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera yaw" },
		{ SectionType::Global, "cam_pitch",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera pitch" },
		{ SectionType::Global, "cam_roll",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera roll" },
		{ SectionType::Global, "cam_fov",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera field of view" },
		// Graphic constants
		{ SectionType::Global, "vpWidth",		SectionVariableType::EXPRESSION_VARIABLE,	"Viewport Width" },
		{ SectionType::Global, "vpHeight",		SectionVariableType::EXPRESSION_VARIABLE,	"Viewport Height" },
		{ SectionType::Global, "aspectRatio",	SectionVariableType::EXPRESSION_VARIABLE,	"Viewport Aspect Ratio" },
		// Fbo constants
		{ SectionType::Global, "fbo{}Width",	SectionVariableType::EXPRESSION_VARIABLE,	"Fbo {} Widht" },
		{ SectionType::Global, "fbo{}Height",	SectionVariableType::EXPRESSION_VARIABLE,	"Fbo {} Height "},
		
		{ SectionType::CameraFPS, "fCameraMode",	SectionVariableType::PARAMETER,				"Camera Mode (0:Free Camera, 1:Only spline, 2:Only formulas, 3:Spline+Formulas" },
		{ SectionType::CameraFPS, "cCameraFile",	SectionVariableType::SPLINE,				"Spline file that defines the camera movement" },
		{ SectionType::CameraFPS, "PosX",			SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in X axis" },
		{ SectionType::CameraFPS, "PosY",			SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Y axis" },
		{ SectionType::CameraFPS, "PosZ",			SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Z axis" },
		{ SectionType::CameraFPS, "UpX",			SectionVariableType::EXPRESSION_VARIABLE,	"Up vector in X axis" },
		{ SectionType::CameraFPS, "UpY",			SectionVariableType::EXPRESSION_VARIABLE,	"Up vector in Y axis" },
		{ SectionType::CameraFPS, "UpZ",			SectionVariableType::EXPRESSION_VARIABLE,	"Up vector in Z axis" },
		{ SectionType::CameraFPS, "Yaw",			SectionVariableType::EXPRESSION_VARIABLE,	"Yaw angle" },
		{ SectionType::CameraFPS, "Pitch",			SectionVariableType::EXPRESSION_VARIABLE,	"Pitch angle" },
		{ SectionType::CameraFPS, "Roll",			SectionVariableType::EXPRESSION_VARIABLE,	"Roll angle" },
		{ SectionType::CameraFPS, "Fov",			SectionVariableType::EXPRESSION_VARIABLE,	"Fov angle" },
		{ SectionType::CameraFPS, "Near",			SectionVariableType::EXPRESSION_VARIABLE,	"Near plane distance" },
		{ SectionType::CameraFPS, "Far",			SectionVariableType::EXPRESSION_VARIABLE,	"Far plane distance" },

		{ SectionType::CameraTarget, "fCameraMode",	SectionVariableType::PARAMETER,				"Camera Mode (0:Free Camera, 1:Only spline, 2:Only formulas, 3:Spline+Formulas" },
		{ SectionType::CameraTarget, "cCameraFile",	SectionVariableType::SPLINE,				"Spline file that defines the camera movement" },
		{ SectionType::CameraTarget, "PosX",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in X axis" },
		{ SectionType::CameraTarget, "PosY",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Y axis" },
		{ SectionType::CameraTarget, "PosZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Z axis" },
		{ SectionType::CameraTarget, "TargetX",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera target in X axis" },
		{ SectionType::CameraTarget, "TargetY",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera target in Y axis" },
		{ SectionType::CameraTarget, "TargetZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera target in Z axis" },
		{ SectionType::CameraTarget, "Yaw",			SectionVariableType::EXPRESSION_VARIABLE,	"Yaw angle" },
		{ SectionType::CameraTarget, "Pitch",		SectionVariableType::EXPRESSION_VARIABLE,	"Pitch angle" },
		{ SectionType::CameraTarget, "Roll",		SectionVariableType::EXPRESSION_VARIABLE,	"Roll angle" },
		{ SectionType::CameraTarget, "Fov",			SectionVariableType::EXPRESSION_VARIABLE,	"Fov angle" },
		{ SectionType::CameraTarget, "Near",		SectionVariableType::EXPRESSION_VARIABLE,	"Near plane distance" },
		{ SectionType::CameraTarget, "Far",			SectionVariableType::EXPRESSION_VARIABLE,	"Far plane distance" },

		{ SectionType::DrawEmitterScene, "sScene",				SectionVariableType::STRING,				"3D model used as reference, each emitter will be placed on the vertex of the given model" },
		{ SectionType::DrawEmitterScene, "fEmissionTime",		SectionVariableType::PARAMETER,				"Time between emissions (seconds)" },
		{ SectionType::DrawEmitterScene, "fParticleLifetime",	SectionVariableType::PARAMETER,				"Particles lifetime (seconds)" },
		{ SectionType::DrawEmitterScene, "tx",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System position X" },
		{ SectionType::DrawEmitterScene, "ty",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System position Y" },
		{ SectionType::DrawEmitterScene, "tz",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System position Z" },
		{ SectionType::DrawEmitterScene, "rx",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System rotation X" },
		{ SectionType::DrawEmitterScene, "ry",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System rotation Y" },
		{ SectionType::DrawEmitterScene, "rz",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System rotation Z" },
		{ SectionType::DrawEmitterScene, "sx",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System scale X" },
		{ SectionType::DrawEmitterScene, "sy",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System scale Y" },
		{ SectionType::DrawEmitterScene, "sz",					SectionVariableType::EXPRESSION_VARIABLE,	"Particle System scale Z" },
		{ SectionType::DrawEmitterScene, "partSpeed",			SectionVariableType::EXPRESSION_VARIABLE,	"Speed variability (it impacts the emission time)" },
		{ SectionType::DrawEmitterScene, "partRandomness",		SectionVariableType::EXPRESSION_VARIABLE,	"Randomness factor on the generated particles (0 to infnite)" },
		{ SectionType::DrawEmitterScene, "velX",				SectionVariableType::EXPRESSION_VARIABLE,	"Initial velocity X how particles are emitted" },
		{ SectionType::DrawEmitterScene, "velY",				SectionVariableType::EXPRESSION_VARIABLE,	"Initial velocity Y how particles are emitted" },
		{ SectionType::DrawEmitterScene, "velZ",				SectionVariableType::EXPRESSION_VARIABLE,	"Initial velocity Z how particles are emitted" },
		{ SectionType::DrawEmitterScene, "forceX",				SectionVariableType::EXPRESSION_VARIABLE,	"Force X applied to the particles" },
		{ SectionType::DrawEmitterScene, "forceY",				SectionVariableType::EXPRESSION_VARIABLE,	"Force Y applied to the particles" },
		{ SectionType::DrawEmitterScene, "forceZ",				SectionVariableType::EXPRESSION_VARIABLE,	"Force Z applied to the particles" },
		{ SectionType::DrawEmitterScene, "colorR",				SectionVariableType::EXPRESSION_VARIABLE,	"Color Red applied to the particles" },
		{ SectionType::DrawEmitterScene, "colorG",				SectionVariableType::EXPRESSION_VARIABLE,	"Color Green applied to the particles" },
		{ SectionType::DrawEmitterScene, "colorB",				SectionVariableType::EXPRESSION_VARIABLE,	"Color Blue applied to the particles" },
		{ SectionType::DrawEmitterScene, "nE",					SectionVariableType::EXPRESSION_VARIABLE,	"Current Emitter" },
		{ SectionType::DrawEmitterScene, "TnE",					SectionVariableType::EXPRESSION_CONSTANT,	"Total number of Emitters" },

		{ SectionType::DrawEmitterSceneEx, "TBD",				SectionVariableType::STRING,				"TBD" },
		{ SectionType::DrawEmitterSpline, "TBD",				SectionVariableType::STRING,				"TBD" },
		
		{ SectionType::DrawFbo, "fFboNum",						SectionVariableType::PARAMETER,				"Framebuffer number to be used" },
		{ SectionType::DrawFbo, "fFboAttachment",				SectionVariableType::PARAMETER,				"Framebuffer attachment number to be used" },
		{ SectionType::DrawFbo, "fClearScreenBuffer",			SectionVariableType::PARAMETER,				"Clear the screen buffer before restoring?" },
		{ SectionType::DrawFbo, "fClearDepthBuffer",			SectionVariableType::PARAMETER,				"Clear the depth buffer before restoring?" },
		
		{ SectionType::DrawFbo2, "fFboNum",						SectionVariableType::PARAMETER,				"Framebuffer number to be used" },
		{ SectionType::DrawFbo2, "fFboAttachment",				SectionVariableType::PARAMETER,				"Framebuffer attachment number to be used" },
		{ SectionType::DrawFbo2, "fClearScreenBuffer",			SectionVariableType::PARAMETER,				"Clear the screen buffer before restoring?" },
		{ SectionType::DrawFbo2, "fClearDepthBuffer",			SectionVariableType::PARAMETER,				"Clear the depth buffer before restoring?" },
		{ SectionType::DrawFbo2, "fFullscreen",					SectionVariableType::PARAMETER,				"Draw FBO at fullscreen?" },
		{ SectionType::DrawFbo2, "fFitToContent",				SectionVariableType::PARAMETER,				"Fit to content (keep aspect ratio) or fill the screen?" },
		{ SectionType::DrawFbo2, "fFilter",						SectionVariableType::PARAMETER,				"Use bilinear filter?" },
		{ SectionType::DrawFbo2, "tx",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO position X" },
		{ SectionType::DrawFbo2, "ty",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO position Y" },
		{ SectionType::DrawFbo2, "tz",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO position Z" },
		{ SectionType::DrawFbo2, "rx",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO rotation X" },
		{ SectionType::DrawFbo2, "ry",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO rotation Y" },
		{ SectionType::DrawFbo2, "rz",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO rotation Z" },
		{ SectionType::DrawFbo2, "sx",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO scale X" },
		{ SectionType::DrawFbo2, "sy",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO scale Y" },
		{ SectionType::DrawFbo2, "sz",							SectionVariableType::EXPRESSION_VARIABLE,	"FBO scale Z" },
	};
	
	const std::size_t kSectionVariableInfoCount = sizeof(kSectionVariableInfo) / sizeof(SectionVariable);
}
