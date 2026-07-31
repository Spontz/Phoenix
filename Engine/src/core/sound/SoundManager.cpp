// SoundManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/sound/SoundManager.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace Phoenix {

	namespace {
		float normalizeBandPower(float power)
		{
			if (!std::isfinite(power) || power <= 0.0f)
				return 0.0f;

			constexpr float dbFloor = -60.0f;
			const float rms = std::sqrt(power);
			const float dbfs = 20.0f * std::log10(rms);
			return std::clamp((dbfs - dbFloor) / -dbFloor, 0.0f, 1.0f);
		}
	}

	SoundManager::SoundManager()
		:
		m_channels(CHANNEL_COUNT),
		m_sampleRate(SAMPLE_RATE),
		m_pDevice(nullptr),
		m_pOutputFFTF32(nullptr),
		m_pSampleBuf(nullptr),
		m_pFFTBuffer(nullptr),
		m_pFFTFrequencies(nullptr),
		m_forceLoad(false)
	{
		sound.clear();
		m_LoadedSounds = 0;
		m_inited = false;

	}

	SoundManager::~SoundManager()
	{
		ma_event_signal(&m_stopEvent);	// Send the signal to stop
		ma_event_wait(&m_stopEvent);	// Wait the stop
		destroyDevice();
		clearSounds();
		kiss_fft_free(m_fftcfg);		// Free fft

		// Delete internal buffers
		if (m_pSampleBuf)
			free(m_pSampleBuf);
		if (m_pOutputFFTF32)
			free(m_pOutputFFTF32);
		if (m_pFFTBuffer)
			free(m_pFFTBuffer);
		if (m_pFFTFrequencies)
			free(m_pFFTFrequencies);
	}

	void SoundManager::destroyDevice()
	{
		if (m_pDevice) {
			ma_device_stop(m_pDevice);
			ma_device_uninit(m_pDevice);
			free(m_pDevice);
			m_pDevice = nullptr;
		}
	}

	bool SoundManager::init()
	{
		ma_result result;

		if (m_inited)
			return false;
		
		// Setup FFT variables
		// Sample buffer
		m_pSampleBuf = (float*)malloc(sizeof(float) * FFT_SIZE * 2);
		if (m_pSampleBuf)
			memset(m_pSampleBuf, 0, sizeof(float) * FFT_SIZE * 2);

		// FFT config
		m_fftcfg = kiss_fftr_alloc(FFT_SIZE * 2, false, NULL, NULL);

		// FFT values buffer
		m_pFFTBuffer = (float*)malloc(sizeof(float) * FFT_SIZE);
		if (m_pFFTBuffer)
			memset(m_pFFTBuffer, 0, sizeof(float) * FFT_SIZE);

		// FFT frequencies buffer
		m_pFFTFrequencies = (float*)malloc(sizeof(float) * FFT_SIZE);
		if (m_pFFTFrequencies) {
			for (int32_t i = 0; i < FFT_SIZE; i++) {
				m_pFFTFrequencies[i] = static_cast<float>(i) * (SAMPLE_RATE / 2.0f / FFT_SIZE);
			}
		}

		// FFT output buffer
		m_pOutputFFTF32 = (float*)malloc(sizeof(float) * SAMPLE_STORAGE);
		if (m_pOutputFFTF32)
			memset(m_pOutputFFTF32, 0, sizeof(float) * SAMPLE_STORAGE);

		// Allocate space for structure
		m_pDevice = (ma_device*)malloc(sizeof(ma_device));

		// Init the device
		ma_device_config deviceConfig;
		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format = SAMPLE_FORMAT;
		deviceConfig.playback.channels = m_channels;
		deviceConfig.sampleRate = m_sampleRate;
		deviceConfig.dataCallback = dataCallback;
		deviceConfig.pUserData = this;

		result = ma_device_init(NULL, &deviceConfig, m_pDevice);
		if (result != MA_SUCCESS) {
			// Failed to open playback device
			Logger::error("Error initing the sound device");
			destroyDevice();
			m_inited = false;
			return m_inited;
		}

		// We can't stop in the audio thread so we instead need to use an event. We wait on this thread in the main thread, and signal it in the audio thread. This
		// needs to be done before starting the device. We need a context to initialize the event, which we can get from the device. Alternatively you can initialize
		// a context separately, but we don't need to do that for this example.
		ma_event_init(&m_stopEvent);

		m_inited = true;
		return m_inited;
	}

	bool SoundManager::setMasterVolume(float volume)
	{
		if (m_inited && m_pDevice) {
			ma_device_set_master_volume(m_pDevice, volume);
			return true;
		}
		return false;
	}

	SP_Sound SoundManager::addSound(const std::string_view filePath)
	{
		SP_Sound p_sound;
		std::lock_guard lock(m_soundListMutex);

		// check if sound is already loaded, then we just retrieve the ID of our sound
		for (auto const& m_sound : sound) {
			if (m_sound->filePath.compare(filePath) == 0) {
				p_sound = m_sound;
			}
		}

		if (p_sound == nullptr) {
			SP_Sound new_sound = std::make_shared<Sound>();
			if (new_sound->loadSoundFile(filePath, m_channels, m_sampleRate)) {
				sound.emplace_back(new_sound);
				m_LoadedSounds++;
				p_sound = new_sound;
				Logger::info(LogLevel::med, "Song {} loaded OK", filePath);
			}
			else {
				Logger::error("Could not load song: {}", filePath);
			}
		}
		else { // If the song is catched we should not do anything, unless we have been told to upload it again
			if (m_forceLoad) {
				if (p_sound->loadSoundFile(filePath, m_channels, m_sampleRate)) {
					Logger::info(LogLevel::med, "Song {} force reload OK", filePath);
				}
				else
					Logger::error("Could not load song: {}", filePath);
			}
		}

		return p_sound;
	}

	SP_Sound SoundManager::getSoundbyID(uint32_t id)
	{
		std::lock_guard lock(m_soundListMutex);
		if (id >= sound.size())
			return nullptr;
		else
			return sound[id];
	}

	void SoundManager::clearSounds()
	{
		std::lock_guard lock(m_soundListMutex);
		sound.clear();
		m_LoadedSounds = 0;
		resetBeatDetection();
	}

	std::string SoundManager::getVersion()
	{
		std::string ma_version;
		ma_version = MA_VERSION_STRING;
		return ma_version;
	}

	void SoundManager::playDevice()
	{
		if (m_pDevice)
			ma_device_start(m_pDevice);
	}

	void SoundManager::stopDevice()
	{
		if (m_pDevice)
			ma_device_stop(m_pDevice);
	}

	void SoundManager::setStreamingAudioSink(std::function<bool(const float*, uint32_t, uint32_t, uint32_t)> sink)
	{
		std::lock_guard lock(m_streamingAudioSinkMutex);
		m_streamingAudioSink = std::move(sink);
	}

	void SoundManager::stopAllSounds()
	{
		std::vector<SP_Sound> soundsSnapshot;
		{
			std::lock_guard lock(m_soundListMutex);
			soundsSnapshot = sound;
		}

		for (auto const& m_sound : soundsSnapshot) {
			m_sound->stopSound();
		}
		resetBeatDetection();
	}

	void SoundManager::resetBeatDetection()
	{
		{
			std::lock_guard lock(m_fftSampleMutex);
			if (m_pSampleBuf)
				memset(m_pSampleBuf, 0, sizeof(float) * FFT_SIZE * 2);
		}
		m_fBeatBaseline = 0.0f;
		m_beatBaselineInitialized = false;
		m_beatArmed = true;
		m_fBeat = 0.0f;
	}

	void SoundManager::enumerateDevices()
	{
		ma_result result;
		ma_context context;
		ma_device_info* pPlaybackDeviceInfos;
		ma_uint32 playbackDeviceCount;
		ma_device_info* pCaptureDeviceInfos;
		ma_uint32 captureDeviceCount;
		ma_uint32 iDevice;

		if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
			Logger::error("Failed to initialize context");
			return;
		}

		result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
		if (result != MA_SUCCESS) {
			Logger::error("Failed to retrieve device information");
			return;
		}

		Logger::info(LogLevel::low, "Sound playback Devices detected:");
		for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
			Logger::info(LogLevel::low, "Device: {}: {}", iDevice, pPlaybackDeviceInfos[iDevice].name);
		}
		
		ma_context_uninit(&context);
	}

	void SoundManager::dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
	{
		(void)pInput; // added to avoid compiler warnings. It does nothing.
		float* pOutputF32 = (float*)pOutput;
		SoundManager* p_sm = (SoundManager*)pDevice->pUserData;
		memset(p_sm->m_pOutputFFTF32, 0, sizeof(float) * SAMPLE_STORAGE);

		std::vector<SP_Sound> soundsSnapshot;
		{
			std::lock_guard lock(p_sm->m_soundListMutex);
			soundsSnapshot = p_sm->sound;
		}

		for (auto const& mySound : soundsSnapshot) {
			if (!mySound->isPlaying())
				continue;

			const ma_uint32 framesRead = mySound->mixFrames(pOutputF32, p_sm->m_pOutputFFTF32, frameCount, p_sm->m_channels);
			if (framesRead < frameCount)
				mySound->stopSound();
		}

		std::function<bool(const float*, uint32_t, uint32_t, uint32_t)> streamingAudioSink;
		{
			std::lock_guard lock(p_sm->m_streamingAudioSinkMutex);
			streamingAudioSink = p_sm->m_streamingAudioSink;
		}
		if (streamingAudioSink) {
			const bool sentToPreview = streamingAudioSink(pOutputF32, frameCount, p_sm->m_channels, p_sm->m_sampleRate);
			if (sentToPreview)
				memset(pOutputF32, 0, sizeof(float) * frameCount * p_sm->m_channels);
		}

		// Fill the sampleBuffer for the FFT analysis
		frameCount = frameCount < FFT_SIZE * 2 ? frameCount : FFT_SIZE * 2;

		// Just rotate the buffer; copy existing, append new - https://github.com/Gargaj/Bonzomatic/blob/master/src/platform_common/FFT.cpp
		const float* samples = (const float*)p_sm->m_pOutputFFTF32;
		if (samples) {
			std::unique_lock lock(p_sm->m_fftSampleMutex, std::try_to_lock);
			if (lock.owns_lock()) {
				float* p_sample = p_sm->m_pSampleBuf;
				for (uint32_t i = frameCount; i < (FFT_SIZE * 2); i++) {
					*(p_sample++) = p_sm->m_pSampleBuf[i];
				}
				for (uint32_t i = 0; i < frameCount; i++) {
					*(p_sample++) = (samples[i * 2] + samples[i * 2 + 1]) / 2.0f * p_sm->m_fAmplification;
				}
			}
		}
	}

	bool SoundManager::performFFT(float frameTime)
	{
		if (!m_inited) {
			return false;
		}

		std::array<float, FFT_SIZE * 2> samples;
		{
			std::lock_guard lock(m_fftSampleMutex);
			std::copy_n(m_pSampleBuf, samples.size(), samples.begin());
		}

		kiss_fft_cpx out[FFT_SIZE + 1];			// FFT complex output
		kiss_fftr(m_fftcfg, samples.data(), out);


		m_fLowFreqLevel = 0.0f;
		m_fMidFreqLevel = 0.0f;
		m_fHighFreqLevel = 0.0f;
		float beatMagnitude = 0.0f;
		float lowBandPower = 0.0f;
		float midBandPower = 0.0f;
		float highBandPower = 0.0f;

		constexpr float fftLength = static_cast<float>(FFT_SIZE * 2);
		constexpr float inverseFftLengthSquared = 1.0f / (fftLength * fftLength);

		for (uint32_t i = 0; i < FFT_SIZE; i++)
		{
			// Calculate the FFT buffer
			static const float scaling = 1.0f / static_cast<float>(FFT_SIZE);
			m_pFFTBuffer[i] = 2.0f * sqrtf(out[i].r * out[i].r + out[i].i * out[i].i) * scaling;

			if (i == 0)
				continue;

			const float magnitudeSquared = out[i].r * out[i].r + out[i].i * out[i].i;
			const float power = 2.0f * magnitudeSquared * inverseFftLengthSquared;

			if (m_pFFTFrequencies[i] <= m_lowFreqMax) {
				lowBandPower += power;
				beatMagnitude += m_pFFTBuffer[i];
			}
			else if (m_pFFTFrequencies[i] <= m_midFreqMax) {
				midBandPower += power;
			}
			else {
				highBandPower += power;
			}
		}

		const float nyquistMagnitudeSquared =
			out[FFT_SIZE].r * out[FFT_SIZE].r +
			out[FFT_SIZE].i * out[FFT_SIZE].i;
		highBandPower += nyquistMagnitudeSquared * inverseFftLengthSquared;

		m_fLowFreqLevel = normalizeBandPower(lowBandPower);
		m_fMidFreqLevel = normalizeBandPower(midBandPower);
		m_fHighFreqLevel = normalizeBandPower(highBandPower);

		const float safeFrameTime = std::isfinite(frameTime) ? std::clamp(frameTime, 0.0f, 0.25f) : 0.0f;
		const float fadeOut = std::max(m_fFadeOut, 0.0f);
		m_fBeat = std::clamp(m_fBeat - fadeOut * safeFrameTime, 0.0f, 1.0f);

		if (!std::isfinite(beatMagnitude))
			beatMagnitude = 0.0f;

		constexpr float silenceThreshold = 0.000001f;
		constexpr float baselineResponseSeconds = 1.0f;
		const float baselineBlend = 1.0f - std::exp(-safeFrameTime / baselineResponseSeconds);

		// Seed the baseline from one valid window so startup silence cannot generate a beat.
		if (!m_beatBaselineInitialized) {
			if (beatMagnitude > silenceThreshold) {
				m_fBeatBaseline = beatMagnitude;
				m_beatBaselineInitialized = true;
			}
			return true;
		}

		const float baseline = std::max(m_fBeatBaseline, silenceThreshold);
		const float attackRatio = std::max(m_fBeatRatio, silenceThreshold);
		const float rearmRatio = attackRatio * 0.8f;
		const float magnitudeRatio = beatMagnitude / baseline;

		if (m_beatArmed && magnitudeRatio > attackRatio) {
			m_fBeat = 1.0f;
			m_beatArmed = false;
		}
		else if (!m_beatArmed && magnitudeRatio < rearmRatio) {
			m_beatArmed = true;
		}

		m_fBeatBaseline += (beatMagnitude - m_fBeatBaseline) * baselineBlend;
		if (!std::isfinite(m_fBeatBaseline) || m_fBeatBaseline < 0.0f)
			m_fBeatBaseline = 0.0f;

		return true;
	}

	void SoundManager::fillSpectrogram()
	{
		m_spectrogram.fill(m_pFFTBuffer, FFT_SIZE);
	}

}
