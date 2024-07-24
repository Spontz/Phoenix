// Sound.cpp
// Spontz Demogroup

#include "main.h"
#include "core/sound/Sound.h"

namespace Phoenix {

	Sound::Sound()
		:
		m_pDecoder(nullptr),
		filePath(""),
		status(State::NotReady),
		volume (1.0)
	{
	}

	Sound::~Sound()
	{
		unLoadSong();
	}

	void Sound::unLoadSong()
	{
		if (m_pDecoder) {
			ma_decoder_uninit(m_pDecoder);
			free(m_pDecoder);
			m_pDecoder = nullptr;
		}
		status = State::NotReady;
	}

	bool Sound::loadSoundFile(const std::string_view soundFile, uint32_t channels, uint32_t sampleRate)
	{
		ma_result result;

		// If song is already loaded, we unload it first
		if (status != State::NotReady) {
			unLoadSong();
		}
		filePath = soundFile;
		
		// Allocate space for structure
		m_pDecoder = (ma_decoder*)malloc(sizeof(ma_decoder));

		// Init de Decoder and load song
		ma_decoder_config decoderConfig;
		decoderConfig = ma_decoder_config_init(ma_format_f32, channels, sampleRate);
		result = ma_decoder_init_file(soundFile.data(), &decoderConfig, m_pDecoder);
		if (result != MA_SUCCESS) {
			unLoadSong();
			return false;
		}
		
		status = State::Stopped;
		return true;
	}

	bool Sound::playSound()
	{
		if (status != State::NotReady) {
			status = State::Playing;
			return true;
		}
		else
			return false;
	}

	bool Sound::stopSound()
	{
		if (status != State::NotReady) {
			status = State::Stopped;
			return true;
		}
		else
			return false;
	}

	bool Sound::restartSound()
	{
		ma_result result;
		if (status != State::NotReady) {
			result = ma_decoder_seek_to_pcm_frame(m_pDecoder, 0);
			if (result != MA_SUCCESS) {
				return false;
			}
		}
		else
			return false;

		return true;
	}

	void Sound::seekSound(float second)
	{
		if (status != State::NotReady) {
			float myFFrame = static_cast<float>(m_pDecoder->outputSampleRate) * second;
			uint64_t myFrame = static_cast<uint64_t>(myFFrame);
			ma_decoder_seek_to_pcm_frame(m_pDecoder, myFrame);
		}
	}
	ma_decoder* Sound::getDecoder()
	{
		return m_pDecoder;
	}
}