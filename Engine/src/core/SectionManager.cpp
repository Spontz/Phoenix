// SectionManager.cpp
// Spontz Demogroup

#include "main.h"
#include "SectionManager.h"
#include "sections/sections.h"

namespace Phoenix {

	struct SectionInfo final {
		std::string m_id;
		SectionType m_type;
		std::function<Section*()> m_fnCreateSection;
	};

	// sections functions references
	const SectionInfo kSectionInfo[] = {
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

	void SectionManager::loadSectionVariables()
	{
		if (!m_SectionVariableInfo.empty())
			m_SectionVariableInfo.clear();

		// Variables common to all secions
		m_SectionVariableInfo.push_back({ SectionType::Global, "t",				SectionVariableType::EXPRESSION_VARIABLE,	"Current time of the section, time is relative to the section" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "tend",			SectionVariableType::EXPRESSION_VARIABLE,	"Time when the section ends" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "tdemo",			SectionVariableType::EXPRESSION_VARIABLE,	"Global demo time" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "fps",			SectionVariableType::EXPRESSION_VARIABLE,	"FPS of the demo" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "mouseX",		SectionVariableType::EXPRESSION_VARIABLE,	"X value of the mouse position" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "mouseY",		SectionVariableType::EXPRESSION_VARIABLE,	"Y value of the mouse position" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "mouseY",		SectionVariableType::EXPRESSION_VARIABLE,	"Y value of the mouse position" });
		// Multi-purpose variables
		for (int32_t i = 0; i < MULTIPURPOSE_VARS; i++) {
			std::string varNum = "var" + std::to_string(i);
			m_SectionVariableInfo.push_back({ SectionType::Global, varNum,		SectionVariableType::EXPRESSION_VARIABLE,	"Multipurpose variable" });
		}
		// Track beats
		m_SectionVariableInfo.push_back({ SectionType::Global, "beat",		SectionVariableType::EXPRESSION_VARIABLE,	"Sount output beat detection" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "high_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,	"Sum of high frequency sound output" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "mid_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,	"Sum of mid frequency sound output" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "low_freq_sum",	SectionVariableType::EXPRESSION_VARIABLE,	"Sum of low frequency sound output" });

		// Camera values
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_posX",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera position X" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_posY",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera position Y" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_posZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera position Z" });

		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_upX",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera up vector X" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_upY",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera up vector Y" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_upZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera up vector Z" });

		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_yaw",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera yaw" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_pitch",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera pitch" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_roll",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera roll" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "cam_fov",		SectionVariableType::EXPRESSION_VARIABLE,	"Current camera field of view" });

		// Light values
		for (int32_t i = 0; i < DEMO->m_lightManager.light.size(); ++i) {
			std::string lightNum = "light" + std::to_string(i);
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_posX",		SectionVariableType::EXPRESSION_VARIABLE,	"Light position X" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_posY",		SectionVariableType::EXPRESSION_VARIABLE,	"Light position Y" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_posZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Light position Z" });
			
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_dirX",		SectionVariableType::EXPRESSION_VARIABLE,	"Light direction X" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_dirY",		SectionVariableType::EXPRESSION_VARIABLE,	"Light direction Y" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_dirZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Light direction Z" });

			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colAmbR",	SectionVariableType::EXPRESSION_VARIABLE,	"Light ambient color R" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colAmbG",	SectionVariableType::EXPRESSION_VARIABLE,	"Light ambient color G" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colAmbB",	SectionVariableType::EXPRESSION_VARIABLE,	"Light ambient color B" });

			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colDifR",	SectionVariableType::EXPRESSION_VARIABLE,	"Light diffuse color R" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colDifG",	SectionVariableType::EXPRESSION_VARIABLE,	"Light diffuse color G" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colDifB",	SectionVariableType::EXPRESSION_VARIABLE,	"Light diffuse color B" });

			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colSpcR",	SectionVariableType::EXPRESSION_VARIABLE,	"Light specular color R" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colSpcG",	SectionVariableType::EXPRESSION_VARIABLE,	"Light specular color G" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_colSpcB",	SectionVariableType::EXPRESSION_VARIABLE,	"Light specular color B" });

			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_ambientStrength",	SectionVariableType::EXPRESSION_VARIABLE,	"Light ambient strength" });
			m_SectionVariableInfo.push_back({ SectionType::Global, lightNum + "_specularStrength",	SectionVariableType::EXPRESSION_VARIABLE,	"Light specular strength" });
		}

		// Graphic constants
		m_SectionVariableInfo.push_back({ SectionType::Global, "vpWidth",		SectionVariableType::EXPRESSION_VARIABLE,	"Viewport Width" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "vpHeight",		SectionVariableType::EXPRESSION_VARIABLE,	"Viewport Height" });
		m_SectionVariableInfo.push_back({ SectionType::Global, "aspectRatio",	SectionVariableType::EXPRESSION_VARIABLE,	"Viewport Aspect Ratio" });

		// Fbo constants
		for (int32_t i = 0; i < DEMO->m_fboManager.fbo.size(); ++i) {
			std::string fboNumWidth = "fbo" + std::to_string(i) + "Width";
			std::string fboNumHeight = "fbo" + std::to_string(i) + "Height";
			m_SectionVariableInfo.push_back({ SectionType::Global, fboNumWidth,	SectionVariableType::EXPRESSION_VARIABLE,	"Fbo Widht " + i });
			m_SectionVariableInfo.push_back({ SectionType::Global, fboNumHeight,SectionVariableType::EXPRESSION_VARIABLE,	"Fbo Height " + i });
		}

		// Camera FPS
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "fCameraMode",	SectionVariableType::PARAMETER,				"Camera Mode (0:Free Camera, 1:Only spline, 2:Only formulas, 3:Spline+Formulas" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "cCameraFile",	SectionVariableType::SPLINE,				"Spline file that defines the camera movement" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "PosX",			SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in X axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "PosY",			SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Y axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "PosZ",			SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Z axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "UpX",			SectionVariableType::EXPRESSION_VARIABLE,	"Up vector in X axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "UpY",			SectionVariableType::EXPRESSION_VARIABLE,	"Up vector in Y axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "UpZ",			SectionVariableType::EXPRESSION_VARIABLE,	"Up vector in Z axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "Yaw",			SectionVariableType::EXPRESSION_VARIABLE,	"Yaw angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "Pitch",			SectionVariableType::EXPRESSION_VARIABLE,	"Pitch angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "Roll",			SectionVariableType::EXPRESSION_VARIABLE,	"Roll angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "Fov",			SectionVariableType::EXPRESSION_VARIABLE,	"Fov angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "Near",			SectionVariableType::EXPRESSION_VARIABLE,	"Near plane distance" });
		m_SectionVariableInfo.push_back({ SectionType::CameraFPS, "Far",			SectionVariableType::EXPRESSION_VARIABLE,	"Far plane distance" });

		// Camera Target
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "fCameraMode",	SectionVariableType::PARAMETER,				"Camera Mode (0:Free Camera, 1:Only spline, 2:Only formulas, 3:Spline+Formulas" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "cCameraFile",	SectionVariableType::SPLINE,				"Spline file that defines the camera movement" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "PosX",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in X axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "PosY",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Y axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "PosZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera position in Z axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "TargetX",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera target in X axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "TargetY",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera target in Y axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "TargetZ",		SectionVariableType::EXPRESSION_VARIABLE,	"Camera target in Z axis" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "Yaw",			SectionVariableType::EXPRESSION_VARIABLE,	"Yaw angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "Pitch",		SectionVariableType::EXPRESSION_VARIABLE,	"Pitch angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "Roll",		SectionVariableType::EXPRESSION_VARIABLE,	"Roll angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "Fov",			SectionVariableType::EXPRESSION_VARIABLE,	"Fov angle" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "Near",		SectionVariableType::EXPRESSION_VARIABLE,	"Near plane distance" });
		m_SectionVariableInfo.push_back({ SectionType::CameraTarget, "Far",			SectionVariableType::EXPRESSION_VARIABLE,	"Far plane distance" });
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
		m_SectionVariableInfo.clear();
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
