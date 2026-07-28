// Sound.cpp
// Spontz Demogroup

#include "main.h"
#include "core/sound/Sound.h"
#include <vector>

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
		std::lock_guard lock(m_mutex);
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
		std::lock_guard lock(m_mutex);
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
		std::lock_guard lock(m_mutex);
		if (status != State::NotReady) {
			status = State::Playing;
			return true;
		}
		else
			return false;
	}

	bool Sound::stopSound()
	{
		std::lock_guard lock(m_mutex);
		if (status != State::NotReady) {
			status = State::Stopped;
			return true;
		}
		else
			return false;
	}

	bool Sound::restartSound()
	{
		std::lock_guard lock(m_mutex);
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
		std::lock_guard lock(m_mutex);
		if (status != State::NotReady) {
			float myFFrame = static_cast<float>(m_pDecoder->outputSampleRate) * second;
			uint64_t myFrame = static_cast<uint64_t>(myFFrame);
			ma_decoder_seek_to_pcm_frame(m_pDecoder, myFrame);
		}
	}

	bool Sound::isPlaying()
	{
		std::lock_guard lock(m_mutex);
		return status == State::Playing;
	}

	ma_uint32 Sound::mixFrames(float* pOutputF32, float* pOutputFFTF32, ma_uint32 frameCount, ma_uint32 channels)
	{
		std::lock_guard lock(m_mutex);
		if (status != State::Playing || m_pDecoder == nullptr)
			return frameCount;

		const ma_uint32 safeChannels = channels == 0 ? 1 : channels;
		std::vector<float> temp(4096);
		ma_uint32 tempCapInFrames = static_cast<ma_uint32>(temp.size() / safeChannels);
		if (tempCapInFrames == 0)
			tempCapInFrames = 1;
		ma_uint32 totalFramesRead = 0;

		while (totalFramesRead < frameCount) {
			ma_uint64 framesReadThisIteration = 0;
			const ma_uint32 totalFramesRemaining = frameCount - totalFramesRead;
			ma_uint32 framesToReadThisIteration = tempCapInFrames;
			if (framesToReadThisIteration > totalFramesRemaining)
				framesToReadThisIteration = totalFramesRemaining;

			const ma_result result = ma_decoder_read_pcm_frames(m_pDecoder, temp.data(), framesToReadThisIteration, &framesReadThisIteration);
			if (result != MA_SUCCESS || framesReadThisIteration == 0)
				break;

			for (ma_uint64 iSample = 0; iSample < framesReadThisIteration * safeChannels; ++iSample) {
				const ma_uint64 iOutputSample = static_cast<ma_uint64>(totalFramesRead) * safeChannels + iSample;
				pOutputF32[iOutputSample] += temp[iSample] * volume;
				pOutputFFTF32[iOutputSample] += temp[iSample];
			}

			totalFramesRead += static_cast<ma_uint32>(framesReadThisIteration);
			if (framesReadThisIteration < framesToReadThisIteration)
				break;
		}

		return totalFramesRead;
	}

	ma_decoder* Sound::getDecoder()
	{
		std::lock_guard lock(m_mutex);
		return m_pDecoder;
	}
	std::string Sound::getStatusStr()
	{
		std::lock_guard lock(m_mutex);
		std::string s;

		switch (status) {
		case State::NotReady:
			s = "Not Ready";
			break;
		case State::Playing:
			s = "Playing";
			break;
		case State::Stopped:
			s = "Stopped";
			break;
		case State::Finished:
			s = "Finished";
			break;
		default:
			s = "Unknown!";
			break;
		}

		return s;
	}
}