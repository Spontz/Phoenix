#include "main.h"

namespace Phoenix {

	class sSound final : public Section {
	public:
		sSound();
		~sSound();

	public:
		bool		load();
		void		init();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		float		m_fVolume = 1;
		SP_Sound	m_pSound = nullptr;

	};

	Section* instance_sound() {
		return new sSound();
	}

	sSound::sSound()
	{
		type = SectionType::Sound;
	}

	sSound::~sSound()
	{
		if (!m_demo.m_sound)
			return;
		if (m_pSound)
			m_pSound->stopSound();
	}

	bool sSound::load() {
		if (!m_demo.m_sound) {
			Logger::info(LogLevel::med, "Sound disabled, skipping sound section {}", identifier);
			return true;
		}

		if (param.size() != 1 || strings.size() != 1) {
			Logger::error("Sound [{}]: 1 param (Volume [0.0 - 1.0]) and 1 string needed (music path)", identifier);
			return false;
		}

		m_fVolume = param[0];
		if (m_fVolume < 0) {
			m_fVolume = 0;
		}
		
		// Load the song
		m_pSound = m_demo.m_soundManager.addSound(m_demo.m_dataFolder + strings[0]);
		if (!m_pSound)
			return false;
		
		// Adjust volume song
		m_pSound->volume = m_fVolume;

		return true;
	}

	void sSound::init() {
		m_pSound->seekSound(runTime);
	}

	void sSound::exec() {
		if (!m_demo.m_sound)
			return;

		if ((m_demo.m_status & DemoStatus::PLAY) && ((runTime >= 0) && (runTime <= duration)) && (enabled == true))
			m_pSound->playSound();
		else
			m_pSound->stopSound();
	}

	void sSound::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "File: " << m_pSound->filePath << std::endl;
		ss << "Volume: " << std::to_string(m_pSound->volume) << std::endl;
		debugStatic = ss.str();
	}

	std::string sSound::debug() {
		std::stringstream ss;
		ss << debugStatic;
		ss << "Status: " << m_pSound->getStatusStr() << std::endl;
		return ss.str();
	}
}