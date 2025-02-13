// SectionManager.cpp
// Spontz Demogroup

#include "main.h"
#include "SectionManager.h"
#include "sections/sections.h"

namespace Phoenix {

	// sections functions references
	const SectionInfo kSectionInfo[] = {
		{"global",							SectionType::Global, nullptr},				// Special section, not used
		{"loading",							SectionType::Loading, instance_loading},

		// built-in sections
		{"cameraFPS",						SectionType::CameraFPS, instance_cameraFPS},
		{"cameraTarget",					SectionType::CameraTarget, instance_cameraTarget},
		{"light",							SectionType::LightSec, instance_light},
		{"drawScene",						SectionType::DrawScene, instance_drawScene},
		{"drawSceneMatrix",					SectionType::DrawSceneMatrix, instance_drawSceneMatrix},
		{"drawSceneMatrixFolder",			SectionType::DrawSceneMatrixFolder, instance_drawSceneMatrixFolder},
		{"drawSceneMatrixInstanced",		SectionType::DrawSceneMatrixInstanced, instance_drawSceneMatrixInstanced},
		{"drawSceneMatrixInstancedFolder",	SectionType::DrawSceneMatrixInstancedFolder, instance_drawSceneMatrixInstancedFolder},
		{"drawImage",						SectionType::DrawImage, instance_drawImage},
		{"drawSkybox",						SectionType::DrawSkybox, instance_drawSkybox},
		{"drawVideo",						SectionType::DrawVideo, instance_drawVideo},
		{"drawQuad",						SectionType::DrawQuad, instance_drawQuad},
		{"drawFbo",							SectionType::DrawFbo, instance_drawFbo},
		{"drawFbo2",						SectionType::DrawFbo2, instance_drawFbo2},
		{"drawParticles",					SectionType::DrawParticles, instance_drawParticles},
		{"drawParticlesFbo",				SectionType::DrawParticlesFbo, instance_drawParticlesFbo},
		{"drawParticlesImage",				SectionType::DrawParticlesImage, instance_drawParticlesImage},
		{"drawParticlesScene",				SectionType::DrawParticlesScene, instance_drawParticlesScene},
		{"drawEmitterScene",				SectionType::DrawEmitterScene, instance_drawEmitterScene},
		{"drawEmitterSceneEx",				SectionType::DrawEmitterSceneEx, instance_drawEmitterSceneEx},
		{"drawEmitterSpline",				SectionType::DrawEmitterSpline, instance_drawEmitterSpline},
		{"sound",							SectionType::Sound, instance_sound},
		{"setExpression",					SectionType::SetExpression, instance_setExpression},
		{"fboBind",							SectionType::FboBind, instance_fboBind},
		{"fboUnbind",						SectionType::FboUnbind, instance_fboUnbind},
		{"efxAccum",						SectionType::EfxAccum, instance_efxAccum},
		{"efxBloom",						SectionType::EfxBloom, instance_efxBloom},
		{"efxBlur",							SectionType::EfxBlur, instance_efxBlur},
		{"efxFader",						SectionType::EfxFader, instance_efxFader},
		{"efxMotionBlur",					SectionType::EfxMotionBlur, instance_efxMotionBlur},
		{"test",							SectionType::Test, instance_test}
	};

	const SectionVariable SectionManager::kSectionVariableInfo[] = {
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
	
	const SectionInfo* getSectionInfo(SectionType type) {
		static std::unordered_map<SectionType, const SectionInfo*> m;
		if (m.empty())
			for (auto const& i : kSectionInfo)
				m[i.m_type] = &i;

		const auto it = m.find(type);

		if (it == m.end())
			return nullptr;

		return it->second;
	}

	const SectionInfo* getSectionInfo(std::string_view id) {
		static std::unordered_map<std::string, const SectionInfo*> m;
		if (m.empty())
			for (auto const& i : kSectionInfo)
				m[i.m_id] = &i;

		const auto it = m.find(std::string(id));
		if (it == m.end())
			return nullptr;

		return it->second;
	}

	SectionManager::~SectionManager()
	{
		Logger::info(LogLevel::med, "Destructing SectionManager...");
		clear();
	}

	// Adds a Section into the queue
	// Returns the section ID or -1 if the section could not be added
	int32_t SectionManager::addSection(SectionType type, std::string_view dataSource, bool enabled)
	{
		const auto pSectionInfo = getSectionInfo(type);
		if (!pSectionInfo)
			return -1;

		const auto pNewSection = pSectionInfo->m_fnCreateSection();
		pNewSection->loaded = FALSE; // By default, the section is not loaded
		pNewSection->enabled = enabled;
		pNewSection->DataSource = dataSource;
		pNewSection->type = type;
		pNewSection->type_str = pSectionInfo->m_id;
		m_section.emplace_back(pNewSection);

		return static_cast<int32_t>(m_section.size()) - 1;
	}

	Section* SectionManager::getSection(std::string_view id) const
	{
		// hack: slow
		for (auto pSection : m_section)
			if (pSection->identifier == id)
				return pSection;

		return nullptr;
	}

	int32_t SectionManager::getSectionIndex(std::string_view id) const
	{
		// hack: slow
		for (size_t i = 0; i < m_section.size(); i++)
			if (m_section[i]->identifier == id)
				return static_cast<int32_t>(i);

		return -1;
	}

	std::vector<SectionInfo> SectionManager::getSectionTypes() const
	{
		std::vector<SectionInfo> SectionTypes;

		for (auto const& section : kSectionInfo) {
			SectionTypes.emplace_back(section);
		}

		return SectionTypes;
	}

	std::vector<SectionVariable> SectionManager::getSectionVariablesInfo(SectionType type) const
	{
		std::vector<SectionVariable> variables;
		for (auto const& var : kSectionVariableInfo)
			if (var.SectionType == type)
				variables.push_back(var);
		return variables;
	}

	void SectionManager::toggleSections(std::vector<std::string> const& ids)
	{
		for (auto const& id : ids) {
			const auto pSection = getSection(id);
			if (pSection) {
				pSection->enabled = !pSection->enabled;
				// In case we are dealing with a sound section and we are enabing it, we must force an init, so it will be re-positioned to the correct time
				if (pSection->enabled && pSection->type == SectionType::Sound)
					pSection->inited = false;
				//Logger::sendEditor("Section toggled: %s", ids[i].c_str());
			}
			else {
				Logger::error("Section NOT toggled: {}", id);
			}
		}
	}

	bool SectionManager::deleteSections(std::vector<std::string> const& ids)
	{
		bool failed = false;

		for (auto const& sectionId : ids)
			if (getSection(sectionId)) {
				int i = getSectionIndex(sectionId);
				if (i>=0)
				{
					delete m_section[i];
					m_section.erase(m_section.begin() + i);
					//Logger::sendEditor("Section %d [layer: %d id: %s type: %s] deleted", i, ds->layer, ds->identifier.c_str(), ds->type_str.c_str());
				}
				else {
					Logger::error("Section NOT deleted: {}", sectionId);
					failed = true;
				}
			}
			else {
				Logger::error("Section NOT deleted: {}", sectionId);
				failed = true;
			}

		return failed;
	}

	// Delete the old section and create a new one with the new parameters
	void SectionManager::updateSection(std::string_view identifier, std::string_view sScript)
	{
		deleteSections({ identifier.data() });
		DEMO->loadScriptFromNetwork(sScript);
	}



	void SectionManager::setSectionsStartTime(std::vector<std::string> const& ids, float startTime)
	{
		Section* ds;
		int32_t id_size = (int32_t)ids.size();
		
		Logger::ScopedIndent _;

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			if (ds) {
				ds->startTime = startTime;
				ds->duration = ds->endTime - ds->startTime;
				// In case we are dealing with a sound section and we are enabing it, we must force an init, so it will be re-positioned to the correct time
				if (ds->enabled && ds->type == SectionType::Sound)
					ds->inited = false;
				// Reload the splines. This way they are recalculated
				for (size_t k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->updateDuration(ds->duration);
					if (!ds->spline[k]->load()) {
						Logger::error("Message from Editor: setSectionsStartTime error");
						Logger::error("Section {} [id: {}, DataSource: {}] not loaded properly!", ids[i], ds->identifier, ds->DataSource);
					}
				}
				//Logger::sendEditor("Section [%s] changed StartTime: %.3f", ds->identifier.c_str(), ds->startTime);
			}
			else {
				Logger::error("Section NOT modified (StartTime): {}", ids[i]);
			}
		}
	}

	void SectionManager::setSectionsEndTime(std::vector<std::string> const& ids, float endTime)
	{
		Section* ds;
		int32_t id_size = (int32_t)ids.size();

		Logger::ScopedIndent _;

		for (int32_t i = 0; i < id_size; i++) {
			ds = getSection(ids[i]);
			if (ds) {
				ds->endTime = endTime;
				ds->duration = ds->endTime - ds->startTime;
				// In case we are dealing with a sound section and we are enabing it, we must force an init, so it will be re-positioned to the correct time
				if (ds->enabled && ds->type == SectionType::Sound)
					ds->inited = false;
				// Reload the splines. This way they are recalculated
				for (size_t k = 0; k < ds->spline.size(); k++) {
					ds->spline[k]->updateDuration(ds->duration);
					if (!ds->spline[k]->load()) {
						Logger::error("Message from Editor: setSectionsEndTime error");
						Logger::error("Section {} [id: {}, DataSource: {}] not loaded properly!", ids[i], ds->identifier, ds->DataSource);
					}
				}
				//Logger::sendEditor("Section [%s] changed EndTime: %.3f", ds->identifier.c_str(), ds->endTime);
			}
			else {
				Logger::error("Section NOT modified (EndTime): {}", ids[i]);
			}
		}
	}

	void SectionManager::setSectionLayer(std::string_view identifier, int32_t new_layer)
	{
		Section* ds;

		ds = getSection(identifier);
		if (ds) {
			//Logger::sendEditor("Section [%s] changed Layer from %d to %d", ds->identifier.c_str(), ds->layer, new_layer);
			ds->layer = new_layer;
		}
		else {
			Logger::error("Section NOT modified (setSectionLayer): {}", identifier);
		}
	}

	void SectionManager::clear()
	{
		for (auto const& pSection : m_section) {
			Logger::info(LogLevel::low, "Deleting section: {}...", pSection->identifier);
			delete pSection;
		}

		m_section.clear();
		m_loadSection.clear();
		m_execRenderSection.clear();
		m_execSoundSection.clear();
	}
	
	SectionType getSectionType(std::string_view key)
	{
		const auto pSectionInfo = getSectionInfo(key);
		return pSectionInfo ? pSectionInfo->m_type : SectionType::NOT_FOUND;
	}

	std::ostream& operator<<(std::ostream& os, SectionType type)
	{
		const auto pSectionInfo = getSectionInfo(type);
		os << pSectionInfo ? pSectionInfo->m_id : "SectionType::NOT_FOUND";
		return os;
	}

	std::string str(SectionType value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

}
