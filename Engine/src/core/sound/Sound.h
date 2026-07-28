// Sound.h
// Spontz Demogroup

#pragma once 

#include "main.h"

#include <stdio.h>
#include <memory>
#include <string>
#include <string_view>
#include <mutex>

namespace Phoenix {

	class Sound;
	using SP_Sound = std::shared_ptr<Sound>;

	class Sound {

	public:
		enum State {
			NotReady = 0,
			Playing,
			Stopped,
			Finished,
		};


	public:
		Sound();
		virtual ~Sound();

	public:
		bool loadSoundFile(const std::string_view soundFile, uint32_t channels, uint32_t sampleRate); // Load sound from file
		bool playSound();
		bool stopSound();
		bool restartSound();
		void seekSound(float second);
		bool isPlaying();
		ma_uint32 mixFrames(float* pOutputF32, float* pOutputFFTF32, ma_uint32 frameCount, ma_uint32 channels);
		ma_decoder* getDecoder();
		std::string getStatusStr();		// Gets Status in string

	private:
		void unLoadSong();	// Unload song

	public:
		std::string		filePath;		// file path
		Sound::State	status;			// Sound status
		float			volume;			// Sound volume (0.0 to 1.0)

	private:
		ma_decoder		*m_pDecoder;	// Internal miniaudio decoder
		std::mutex		m_mutex;

	};
}