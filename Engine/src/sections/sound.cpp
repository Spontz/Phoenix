#pragma comment( lib, "bass" )

#include "main.h"
#include "core/drivers/bassdriver.h"

namespace Phoenix {
#define BUFFER_SAMPLES	512

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
		int			m_beatID = 0;

		HSTREAM		m_hMusicStream;					// Music Stream

		std::string	m_file = "";
		float		m_fLevels[2] = { 0.0f };		// Left and right channels (this feature is not being used by now)

		// Beat parameters
		float		m_fEnergy[BUFFER_SAMPLES] = { 0.0f };
		float		m_fBeatRatio = 1.4f;
		float		m_fFadeOut = 4.0f;
		float		m_fIntensity = 0;
		int			m_iPosition = 1;
		float		m_fVolume = 1;
		float		m_fPrevVolume = 1;	// Previous volume value
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

		if (!BASS_ChannelStop(m_hMusicStream))
			Logger::error("Sound [{}]: BASS_ChannelStop returned error: %i", identifier, BASS_ErrorGetCode());
		if (!BASS_StreamFree(m_hMusicStream))
			Logger::error("Sound [{}]: Music couldnt be freed: %i", identifier, BASS_ErrorGetCode());
	}

	bool sSound::load() {
		if (!m_demo.m_sound) {
			return false;
		}

		if (param.size() != 4 || strings.size() != 1) {
			Logger::error("Sound [{}]: 4 params (Volume [0.0 - 1.0], BeatID, beatRatio and FadeOut) and 1 string needed (music path)", identifier);
			return false;
		}

		m_fVolume = param[0];
		if (m_fVolume < 0) {
			m_fVolume = 0;
		}
		m_fPrevVolume = m_fVolume;

		m_beatID = static_cast<int>(param[1]);
		if (m_beatID < 0 || m_beatID >= MAX_BEATS) {
			Logger::error("Sound [{}]: Beat ID {} not supported, please choose a range between 0 and {}", identifier, m_beatID, MAX_BEATS);
			return false;
		}

		// Beat detection - Init variables
		m_fBeatRatio = param[2];
		m_fFadeOut = param[3];

		// Clean variables
		memset(m_fEnergy, 0, BUFFER_SAMPLES * sizeof(float));
		m_fIntensity = 0;
		m_iPosition = 1;

		m_file = m_demo.m_dataFolder + strings[0];
		m_hMusicStream = BASS_StreamCreateFile(FALSE, m_file.c_str(), 0, 0, BASS_STREAM_PRESCAN);
		if (m_hMusicStream == 0) {
			Logger::error("Sound [{}]: Cannot read file: {} - Error Code: {}", identifier, m_file, BASS_ErrorGetCode());
			return false;
		}
		return true;
	}

	void sSound::init() {
		int BASS_err = 0;

		if (!m_demo.m_sound)
			return;

		if (m_demo.m_status != DemoStatus::PLAY)
			return;

		// Clean variables
		memset(m_fEnergy, 0, BUFFER_SAMPLES * sizeof(float));
		m_fIntensity = 0;
		m_iPosition = 1;

		QWORD bytes = BASS_ChannelSeconds2Bytes(m_hMusicStream, runTime);		// convert seconds to bytes
		if (FALSE == BASS_ChannelSetPosition(m_hMusicStream, bytes, BASS_POS_BYTE)) { // seek there
			BASS_err = BASS_ErrorGetCode();
			if (BASS_err > 0 && BASS_err != BASS_ERROR_POSITION)
				Logger::error("Sound [{}]: BASS_ChannelSetPosition returned error: {}", identifier, BASS_ErrorGetCode());
		}

		if (FALSE == BASS_Start())
			Logger::error("Sound [{}]: BASS_Start returned error: {}", identifier, BASS_ErrorGetCode());

		if (FALSE == BASS_ChannelPlay(m_hMusicStream, FALSE))
			Logger::error("Sound [{}]: BASS_ChannelPlay returned error: {}", identifier, BASS_ErrorGetCode());

		BASS_ChannelSetAttribute(m_hMusicStream, BASS_ATTRIB_VOL, m_fVolume);
	}

	void sSound::exec() {
		float instant, avg;	// Instant energy
		int i;
		float fft[BUFFER_SAMPLES] = { 0.0f }; // 512 samples, because we have used "BASS_DATA_FFT1024"

		// Adjust volume if necessary
		if (m_fVolume != m_fPrevVolume) {
			BASS_ChannelSetAttribute(m_hMusicStream, BASS_ATTRIB_VOL, m_fVolume);
			m_fPrevVolume = m_fVolume;
		}

		// Get a peak level reading for each channel using 20ms of data. At this moment this feature is not being used
		// BASS_ChannelGetLevelEx(m_hMusicStream, m_fLevels, 0.02f, BASS_LEVEL_STEREO | BASS_LEVEL_VOLPAN);

		// FFT analysis
		if (-1 == BASS_ChannelGetData(m_hMusicStream, fft, BASS_DATA_FFT1024)) {	// get the FFT data
			int BASS_err = BASS_ErrorGetCode();
			if ((BASS_err > 0) && (BASS_err != BASS_ERROR_ENDED))
				Logger::error("Sound [{}]: BASS_ChannelGetData returned error: {}", identifier.c_str(), BASS_err);
		}

		BASSDRV->addFFTdata(fft, BUFFER_SAMPLES);

		// Waveform data
		/*
		float fft_transform[BUFFER_SAMPLES] = { 0.0f };
		if (-1 == BASS_ChannelGetData(m_hMusicStream, fft, (BUFFER_SAMPLES * sizeof(float)) | BASS_DATA_FLOAT)) {
		int BASS_err = BASS_ErrorGetCode();
		if ((BASS_err > 0) && (BASS_err != BASS_ERROR_ENDED))
			Logger::error("Sound [%s]: BASS_ChannelGetData returned error: %i", identifier.c_str(), BASS_err);
		}
		// Data transform: In case we have a waveform data: Split info by channels
		for (i = 0; i < BUFFER_SAMPLES / 2; i++) {
			fft_transform[i] = fft[i * 2];	//Channel 1
			fft_transform[BUFFER_SAMPLES / 2 + i] = fft[i * 2 + 1]; // Channel 2
		}
		BASSDRV->copyFFTdata(fft_transform, BUFFER_SAMPLES);
		*/

		instant = 0;
		for (i = 0; i < BUFFER_SAMPLES; i++)
			instant += fft[i];

		// calculate average energy in last samples
		avg = 0;
		for (i = 0; i < BUFFER_SAMPLES; i++) {
			avg += m_fEnergy[i];
		}
		avg /= (float)m_iPosition;

		// instant sample is a beat?
		if ((instant / avg) > m_fBeatRatio) {
			m_fIntensity = 1.0f;
		}
		else if (m_fIntensity > 0) {
			m_fIntensity -= m_fFadeOut * m_demo.m_frameTime;
			if (m_fIntensity < 0) m_fIntensity = 0;
		}

		// updated kernel shared variable
		// to be used by kernel itself or another sections
		m_demo.m_fBeat[m_beatID] += m_fIntensity;
		if (m_demo.m_fBeat[m_beatID] > 1.0) // TODO: We should control this in the sound driver... I think
			m_demo.m_fBeat[m_beatID] = 1.0f;

		// update energy buffer
		if (m_iPosition < BUFFER_SAMPLES) {
			m_fEnergy[m_iPosition - 1] = instant;
			m_iPosition++;
		}
		else {
			for (i = 1; i < BUFFER_SAMPLES; i++) {
				m_fEnergy[i - 1] = m_fEnergy[i];
			}
			m_fEnergy[BUFFER_SAMPLES - 1] = instant;
		}
	}

	void sSound::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "File: " << m_file << std::endl;
		ss << "Beat ID: " << m_beatID << std::endl;
		debugStatic = ss.str();
	}

	std::string sSound::debug() {
		std::stringstream ss;
		ss << debugStatic;
		ss << "Beat value: " << m_fIntensity << std::endl;
		return ss.str();
	}
}